from pathlib import Path

def replace(path, old, new):
 p=Path(path);t=p.read_text();assert old in t,(path,old);p.write_text(t.replace(old,new))
# Public fixture namespaces follow the declared AL namespaces; compatibility aliases belong to tests.
for p in Path('test/gate').glob('*.cpp'):
 t=p.read_text()
 t=t.replace('using agiru::app::tables::ResourceCost;', 'using ResourceCost = agiru::Projects::Resources::Pricing::ResourceCost_Table;')
 t=t.replace('using agiru::app::tables::ResourceCostType;', 'using ResourceCostType = agiru::options::OptionResourceGroupResourceAll;')
 t=t.replace('using agiru::app::tables::ResourceCostCostType;', 'using ResourceCostCostType = agiru::options::OptionFixedPercentExtraLCYExtra;')
 t=t.replace('using agiru::app::codeunits::TransferOldExtTextLines;', 'using TransferOldExtTextLines = agiru::Foundation::ExtendedText::TransferOldExtTextLines_Codeunit;')
 t=t.replace('agiru::app::tables::ResourceCostType','agiru::options::OptionResourceGroupResourceAll')
 t=t.replace('agiru::app::tables::ResourceCostCostType','agiru::options::OptionFixedPercentExtraLCYExtra')
 t=t.replace('agiru::app::tables::ResourceCost','agiru::Projects::Resources::Pricing::ResourceCost_Table')
 if t!=p.read_text():p.write_text(t)
# ResourceCost's option lists are shared types, independent of their declaring table.
p=Path('test/target/ResourceCost.h');t=p.read_text();a=t.index('namespace agiru::app::tables {');b=t.index('class ResourceCost_Table;',a)
t=t[:a]+'namespace agiru::Projects::Resources::Pricing {\n\n'+t[b:]
t=t.replace('using ResourceCost = ResourceCost_Table;\n','')
t=t.replace('#include "type/Option.h"','#include "type/Option.h"\n\n#include "options/Types.h"')
t=t.replace('public:\n','public:\n  using Table<ResourceCost_Table>::operator=;\n\n',1)
t=t.replace('Option<ResourceCostType>','Option<::agiru::options::OptionResourceGroupResourceAll>')
t=t.replace('Option<ResourceCostCostType>','Option<::agiru::options::OptionFixedPercentExtraLCYExtra>')
t=t.replace('agiru::app::tables','agiru::Projects::Resources::Pricing').replace('::ResourceCost>', '::ResourceCost_Table>').replace('::ResourceCost::','::ResourceCost_Table::').replace('::ResourceCost &','::ResourceCost_Table &')
p.write_text(t)
# Declaration metadata is emitted in the definitions translation unit.
p=Path('test/target/ResourceCost.def.cpp');t=p.read_text();t=t.replace('agiru::app::tables','agiru::Projects::Resources::Pricing');import re
t=re.sub(r'\bResourceCost\b','ResourceCost_Table',t).replace('ResourceCost_Table.Table.al','ResourceCost.Table.al').replace('"ResourceCost_Table.h"','"ResourceCost.h"')
t=t.replace('#include "meta/TableDef.h"','#include "meta/TableDef.h"\n#include "runtime/Catalogue.h"\n#include "runtime/Error.h"\n#include "runtime/Table.h"')
t=t.replace('offsetof(ResourceCost_Table, Type)),','offsetof(ResourceCost_Table, Type),\n        Declared{.toolTip = "Specifies the type."}),')
t=t.replace('Declared{.relation = "if (Type = const(Resource)) Resource else if (Type = "\n                             "const(\\"Group(Resource)\\")) \\"Resource Group\\""}', 'NOT_AN_ANCHOR') if False else t
old='''Declared{.relation = "if (Type = const(Resource)) Resource else if (Type = "
                             "const(\\"Group(Resource)\\")) \\"Resource Group\\""}'''
# The parser's relation token serialization is covered independently by RelationGate.
a=t.index('Declared{.relation =');b=t.index('}),',a)
t=t[:a]+'''Declared{.relation = "if ( Type = const ( Resource ) ) Resource else if ( Type = const ( Group(Resource) ) ) Resource Group",
                 .toolTip = "Specifies the code."}'''+t[b+1:]
t=t.replace('Declared{.relationTable = "Work Type", .relation = "Work Type"}', 'Declared{.relationTable = "Work Type", .relation = "Work Type", .toolTip = "Specifies the code for the type of work. You can also assign a unit price to a work type."}')
t=t.replace('offsetof(ResourceCost_Table, CostType)),','offsetof(ResourceCost_Table, CostType), Declared{.toolTip = "Specifies the type of cost."}),')
t=t.replace('Declared{.autoFormatType = "2"}', 'Declared{.toolTip = "Specifies the cost of one unit of the selected item or resource.", .autoFormatType = "2"}',1)
t=t.replace('Declared{.autoFormatType = "2"}', 'Declared{.toolTip = "Specifies the cost of one unit of the item or resource on the line.", .autoFormatType = "2"}',1)
t=t.replace('.caption = ResourceCost_Table::kName,','.caption = "Resource Cost",')
t=t.replace('\n} // namespace agiru::Projects::Resources::Pricing','\nnamespace {\nnamespace ResourceCost_unit {\nconst RegisterTable<ResourceCost_Table> kInCatalogue;\n} // namespace ResourceCost_unit\n} // namespace\n\n} // namespace agiru::Projects::Resources::Pricing')
p.write_text(t)
# Metadata expectations read the metadata output, not the declaration header.
p=Path('test/gate/InitGate.cpp');t=p.read_text().replace('WriteHeader(agiru::al::ParseTable(al), "Gate.Table.al", enums, {}).text','WriteDefinitions(agiru::al::ParseTable(al), "Gate.Table.al", enums)');t=t.replace('offsetof(Gate,','offsetof(Gate_Table,');p.write_text(t)
p=Path('test/gate/GenTableGate.cpp');t=p.read_text()
t=t.replace('afterRename.find("\\"Work Kind Code\\"")', 'agiru::gen::WriteDefinitions(agiru::al::ParseTable(renamed), std::string(kAlPath), {}).find("\\"Work Kind Code\\"")')
t=t.replace('generated.find("FieldNo{};")', 'generated.find("FieldNo_3{};")')
t=t.replace('generated.find(".tableType = TableType::Temporary,")','agiru::gen::WriteDefinitions(agiru::al::ParseTable(declared), std::string(kAlPath), {}).find(".tableType = TableType::Temporary,")')
t=t.replace('generated.find(".name = \\"Name\\", .fields = ResourceCost::kKey1")','generated.find(".name = \\"Name\\", .fields = ResourceCost_Table::kKey1")') if False else t
old='generated.find(".name = \\"Name\\", .fields = ResourceCost::kKey1")'
# Explicit literal replacement with the current metadata owner.
t=t.replace('generated.find(".name = \\"Name\\", .fields = ResourceCost::kKey1")', 'agiru::gen::WriteDefinitions(agiru::al::ParseTable(renamed), std::string(kAlPath), {}).find(".name = \\"Name\\", .fields = ResourceCost_Table::kKey1")')
p.write_text(t)
