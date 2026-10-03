#include "runtime/NumberSequenceStorage.h"

#include "runtime/Database.h"

#include <string_view>

namespace agiru {

namespace {

constexpr std::string_view kRegistry = R"SQL(
CREATE TABLE IF NOT EXISTS agiru_platform.number_sequences_v1 (
  id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  company_specific boolean NOT NULL,
  company text NOT NULL,
  name text NOT NULL,
  UNIQUE (company_specific, company, name),
  CHECK (company_specific OR company = '')
)
)SQL";

constexpr std::string_view kReserve = R"SQL(
CREATE OR REPLACE FUNCTION agiru_platform.reserve_sequence_v1(sequence_id regclass, count integer)
RETURNS TABLE (value bigint, increment bigint)
LANGUAGE plpgsql SET search_path = pg_catalog AS $function$
DECLARE
  properties record;
  previous bigint;
  called boolean;
  first_value numeric;
  final_value numeric;
BEGIN
  IF count < 1 THEN
    RAISE EXCEPTION 'NumberSequence.Range: a range of % values is none', count;
  END IF;
  SELECT * INTO STRICT properties FROM pg_catalog.pg_sequence WHERE seqrelid = sequence_id;
  IF properties.seqcache <> 1 OR properties.seqcycle THEN
    RAISE EXCEPTION 'NumberSequence requires CACHE 1 and NO CYCLE';
  END IF;
  EXECUTE pg_catalog.format('SELECT last_value, is_called FROM %s', sequence_id)
    INTO STRICT previous, called;
  increment := properties.seqincrement;
  first_value := previous::numeric + CASE WHEN called THEN increment::numeric ELSE 0 END;
  final_value := first_value + (count::numeric - 1) * increment::numeric;
  IF first_value < properties.seqmin OR first_value > properties.seqmax OR
     final_value < properties.seqmin OR final_value > properties.seqmax THEN
    RAISE EXCEPTION 'NumberSequence.Range: the requested range exceeds the sequence bounds';
  END IF;
  PERFORM pg_catalog.setval(sequence_id, final_value::bigint, true);
  value := first_value::bigint;
  RETURN NEXT;
END
$function$
)SQL";

constexpr std::string_view kOperate = R"SQL(
CREATE OR REPLACE FUNCTION agiru_platform.number_sequence_v1(
  operation text, sequence_name text, per_company boolean, company_name text,
  seed bigint DEFAULT 0, step bigint DEFAULT 1, count integer DEFAULT 1)
RETURNS TABLE (value bigint, increment bigint, present boolean)
LANGUAGE plpgsql SET search_path = pg_catalog AS $function$
DECLARE
  identity_key bigint;
  entry_id bigint;
  physical_name text;
  sequence_oid regclass;
  mutation boolean;
  allocation boolean;
BEGIN
  IF operation NOT IN ('insert', 'delete', 'restart', 'exists', 'current', 'reserve') THEN
    RAISE EXCEPTION 'NumberSequence: unknown operation %', operation;
  END IF;
  company_name := CASE WHEN per_company THEN company_name ELSE '' END;
  identity_key := pg_catalog.hashtextextended(
    pg_catalog.json_build_array('agiru.sequence.v1', per_company, company_name, sequence_name)::text,
    0) & 9223372036854775806;
  mutation := operation IN ('insert', 'delete', 'restart');
  allocation := operation IN ('current', 'reserve');
  IF mutation THEN
    PERFORM pg_catalog.pg_advisory_xact_lock(identity_key);
  ELSIF allocation THEN
    PERFORM pg_catalog.pg_advisory_xact_lock_shared(identity_key);
  END IF;
  BEGIN
    IF allocation THEN PERFORM pg_catalog.pg_advisory_lock(identity_key | 1); END IF;
    SELECT registry.id INTO entry_id FROM agiru_platform.number_sequences_v1 AS registry
      WHERE registry.company_specific = per_company AND registry.company = company_name
        AND registry.name = sequence_name;
    present := FOUND;
    IF operation = 'insert' THEN
      IF present THEN
        RAISE EXCEPTION 'the number sequence % already exists', sequence_name;
      END IF;
      IF step = 0 THEN
        RAISE EXCEPTION 'NumberSequence.Insert: increment must not be zero';
      END IF;
      INSERT INTO agiru_platform.number_sequences_v1 (company_specific, company, name)
        VALUES (per_company, company_name, sequence_name) RETURNING id INTO entry_id;
      physical_name := 'sequence_' || entry_id;
      EXECUTE pg_catalog.format(
        'CREATE SEQUENCE agiru_platform.%I AS bigint INCREMENT BY %s START WITH %s '
        'MINVALUE -9223372036854775808 MAXVALUE 9223372036854775807 CACHE 1 NO CYCLE',
        physical_name, step, seed);
      present := true;
    ELSIF operation <> 'exists' AND present THEN
      physical_name := 'sequence_' || entry_id;
      sequence_oid := pg_catalog.to_regclass(pg_catalog.format('agiru_platform.%I', physical_name));
      IF sequence_oid IS NULL THEN
        RAISE EXCEPTION 'NumberSequence: registered storage for % is missing', sequence_name;
      END IF;
      CASE operation
        WHEN 'delete' THEN
          EXECUTE pg_catalog.format('DROP SEQUENCE agiru_platform.%I', physical_name);
          DELETE FROM agiru_platform.number_sequences_v1 WHERE id = entry_id;
          present := false;
        WHEN 'restart' THEN
          EXECUTE pg_catalog.format('ALTER SEQUENCE agiru_platform.%I RESTART WITH %s',
            physical_name, seed);
        WHEN 'current' THEN
          EXECUTE pg_catalog.format('SELECT last_value FROM agiru_platform.%I', physical_name)
            INTO STRICT value;
        WHEN 'reserve' THEN
          SELECT reserved.value, reserved.increment INTO value, increment
            FROM agiru_platform.reserve_sequence_v1(sequence_oid, count) AS reserved;
      END CASE;
    END IF;
  EXCEPTION
    WHEN query_canceled OR assert_failure THEN
      IF allocation THEN PERFORM pg_catalog.pg_advisory_unlock(identity_key | 1); END IF;
      RAISE;
    WHEN OTHERS THEN
      IF allocation THEN PERFORM pg_catalog.pg_advisory_unlock(identity_key | 1); END IF;
      RAISE;
  END;
  IF allocation THEN PERFORM pg_catalog.pg_advisory_unlock(identity_key | 1); END IF;
  IF entry_id IS NULL AND operation <> 'exists' THEN RETURN; END IF;
  RETURN NEXT;
END
$function$
)SQL";

}

void ProvisionNumberSequences(const Connection &connection) {
  const Result legacy = connection.Execute(
      "SELECT 1 FROM pg_catalog.pg_class AS relation "
      "JOIN pg_catalog.pg_namespace AS schema ON schema.oid = relation.relnamespace "
      "WHERE relation.relkind = 'S' AND schema.nspname = 'public' "
      "AND relation.relname LIKE 'NumSeq$%' LIMIT 1");
  if (legacy.Rows() != 0) {
    throw DatabaseError("NumberSequence: legacy NumSeq$ storage requires an explicit identity "
                        "migration before provisioning (board:0723)");
  }
  connection.Run("CREATE SCHEMA IF NOT EXISTS agiru_platform");
  connection.Run(kRegistry);
  connection.Run(kReserve);
  connection.Run(kOperate);
}

}
