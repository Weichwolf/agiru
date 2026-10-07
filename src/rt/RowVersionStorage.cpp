#include "runtime/RowVersionStorage.h"

#include "runtime/Database.h"

#include <string_view>

namespace agiru {

namespace {

constexpr std::string_view kSequence = R"SQL(
CREATE SEQUENCE IF NOT EXISTS agiru_platform.rowversions_v1 AS bigint
  INCREMENT BY 1 MINVALUE 1 MAXVALUE 9223372036854775807 START WITH 1
  CACHE 1 NO CYCLE OWNED BY NONE
)SQL";

constexpr std::string_view kValidate = R"SQL(
DO $validation$
BEGIN
  IF NOT EXISTS (
    SELECT 1 FROM pg_catalog.pg_class AS relation
    JOIN pg_catalog.pg_sequence AS properties ON properties.seqrelid = relation.oid
    WHERE relation.oid = 'agiru_platform.rowversions_v1'::regclass
      AND relation.relkind = 'S' AND relation.relpersistence = 'p'
      AND properties.seqtypid = 'pg_catalog.int8'::regtype
      AND properties.seqincrement = 1 AND properties.seqmin = 1
      AND properties.seqmax = 9223372036854775807 AND properties.seqstart = 1
      AND properties.seqcache = 1 AND NOT properties.seqcycle
      AND NOT EXISTS (
        SELECT 1 FROM pg_catalog.pg_depend AS dependency
        WHERE dependency.classid = 'pg_catalog.pg_class'::regclass
          AND dependency.objid = relation.oid AND dependency.deptype IN ('a', 'i'))
  ) THEN
    RAISE EXCEPTION 'RowVersion: incompatible sequence storage; explicit migration required';
  END IF;
END
$validation$
)SQL";

constexpr std::string_view kLast = R"SQL(
CREATE OR REPLACE FUNCTION agiru_platform.last_rowversion_v1()
RETURNS bigint LANGUAGE sql VOLATILE PARALLEL UNSAFE SET search_path = pg_catalog AS $function$
  SELECT CASE WHEN is_called THEN last_value ELSE 0 END FROM agiru_platform.rowversions_v1
$function$
)SQL";

constexpr std::string_view kNext = R"SQL(
CREATE OR REPLACE FUNCTION agiru_platform.next_rowversion_v1()
RETURNS bigint LANGUAGE plpgsql SET search_path = pg_catalog AS $function$
DECLARE
  transaction_tag text := pg_catalog.pg_current_xact_id()::text || ':';
  fence text := pg_catalog.current_setting('agiru.rowversion_fence_v1', true);
  value bigint;
BEGIN
  IF pg_catalog.starts_with(fence, transaction_tag) THEN
    RETURN pg_catalog.nextval('agiru_platform.rowversions_v1'::regclass);
  END IF;
  BEGIN
    PERFORM pg_catalog.pg_advisory_lock(x'41475256'::integer, 1);
    value := pg_catalog.nextval('agiru_platform.rowversions_v1'::regclass);
    PERFORM pg_catalog.pg_advisory_xact_lock(-value);
    PERFORM pg_catalog.set_config('agiru.rowversion_fence_v1', transaction_tag || value::text, true);
    PERFORM pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1);
  EXCEPTION
    WHEN query_canceled OR assert_failure THEN
      PERFORM pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1);
      RAISE;
    WHEN OTHERS THEN
      PERFORM pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1);
      RAISE;
  END;
  RETURN value;
END
$function$
)SQL";

constexpr std::string_view kMinimum = R"SQL(
CREATE OR REPLACE FUNCTION agiru_platform.minimum_rowversion_v1()
RETURNS bigint LANGUAGE plpgsql SET search_path = pg_catalog AS $function$
DECLARE
  value bigint;
BEGIN
  BEGIN
    PERFORM pg_catalog.pg_advisory_lock(x'41475256'::integer, 1);
    SELECT pg_catalog.min(-((classid::bigint << 32) | objid::bigint)) INTO value
      FROM pg_catalog.pg_locks
      WHERE locktype = 'advisory' AND objsubid = 1 AND granted AND mode = 'ExclusiveLock'
        AND classid::bigint >= 2147483648
        AND database = (SELECT oid FROM pg_catalog.pg_database
                        WHERE datname = pg_catalog.current_database());
    IF value IS NULL THEN
      value := agiru_platform.last_rowversion_v1() + 1;
    END IF;
    PERFORM pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1);
  EXCEPTION
    WHEN query_canceled OR assert_failure THEN
      PERFORM pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1);
      RAISE;
    WHEN OTHERS THEN
      PERFORM pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1);
      RAISE;
  END;
  RETURN value;
END
$function$
)SQL";

constexpr std::string_view kWriteTransaction = R"SQL(
CREATE OR REPLACE FUNCTION agiru_platform.write_transaction_v1()
RETURNS uuid LANGUAGE plpgsql VOLATILE PARALLEL UNSAFE SET search_path = pg_catalog AS $function$
DECLARE
  owner_tag text := pg_catalog.pg_current_xact_id()::text || ':';
  held_token text := pg_catalog.current_setting('agiru.write_transaction_v1', true);
  token uuid;
BEGIN
  IF pg_catalog.starts_with(held_token, owner_tag) THEN
    RETURN pg_catalog.substr(held_token, pg_catalog.length(owner_tag) + 1)::uuid;
  END IF;
  token := pg_catalog.gen_random_uuid();
  PERFORM pg_catalog.set_config('agiru.write_transaction_v1', owner_tag || token::text, true);
  RETURN token;
END
$function$
)SQL";

}

void ProvisionRowVersions(const Connection &connection) {
  const bool callerTransaction = connection.InTransaction();
  connection.Run(callerTransaction ? "SAVEPOINT agiru_rowversions_provision_v1" : "BEGIN");
  try {
    connection.Run("SELECT pg_catalog.pg_advisory_xact_lock(x'41475256'::integer, 0)");
    connection.Run("CREATE SCHEMA IF NOT EXISTS agiru_platform");
    connection.Run(kSequence);
    connection.Run(kValidate);
    connection.Run(kLast);
    connection.Run(kNext);
    connection.Run(kMinimum);
    connection.Run(kWriteTransaction);
    connection.Run(callerTransaction ? "RELEASE SAVEPOINT agiru_rowversions_provision_v1"
                                     : "COMMIT");
  } catch (const DatabaseError &) {
    connection.Run(callerTransaction ? "ROLLBACK TO SAVEPOINT agiru_rowversions_provision_v1"
                                     : "ROLLBACK");
    if (callerTransaction) { connection.Run("RELEASE SAVEPOINT agiru_rowversions_provision_v1"); }
    throw;
  }
}

}
