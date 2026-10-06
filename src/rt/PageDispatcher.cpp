#include "runtime/PageDispatcher.h"

#include "meta/PageDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageCore.h"

#include <string>
#include <string_view>

namespace agiru {

namespace {

[[noreturn]] void Refuse(std::string_view control, std::string_view reason) {
  throw Error("Page command for '" + std::string(control) + "': " + std::string(reason),
              "PageCommand");
}

bool WritesControl(PageControlOperation operation) {
  return operation == PageControlOperation::Set || operation == PageControlOperation::Lookup ||
         operation == PageControlOperation::AssistEdit;
}

ControlTriggerKind TriggerFor(PageControlOperation operation) {
  switch (operation) {
    case PageControlOperation::Action: return ControlTriggerKind::Action;
    case PageControlOperation::Lookup: return ControlTriggerKind::Lookup;
    case PageControlOperation::DrillDown: return ControlTriggerKind::DrillDown;
    case PageControlOperation::AssistEdit: return ControlTriggerKind::AssistEdit;
    case PageControlOperation::Read:
    case PageControlOperation::ReadValue:
    case PageControlOperation::Inspect:
    case PageControlOperation::Set:
    case PageControlOperation::Unknown:
    case PageControlOperation::Filter: break;
  }
  throw Error("Unsupported page control operation", "PageCommand");
}

}

PageDispatcher::PageDispatcher(const PageDef &declaration,
                               PageCore &page,
                               PageAuthorization &authorization)
    : declaration_(declaration), page_(page), authorization_(authorization) {}

const ControlDef &PageDispatcher::RequireControl(const PageControlCommand &command) {
  authorization_.Require(declaration_.id, command);
  const ControlDef *control = Control(declaration_.layout, command.control);
  if (control == nullptr) { control = Control(declaration_.actions, command.control); }
  if (control == nullptr) { Refuse(command.control, "unknown declared control"); }
  switch (command.operation) {
    case PageControlOperation::Inspect:
      if (control->kind != ControlKind::Field && control->kind != ControlKind::Action) {
        Refuse(command.control, "unsupported discovery kind");
      }
      break;
    case PageControlOperation::Action:
      if (control->kind != ControlKind::Action) {
        Refuse(command.control, "unsupported action kind");
      }
      break;
    case PageControlOperation::Read:
    case PageControlOperation::ReadValue:
    case PageControlOperation::Set:
    case PageControlOperation::Filter:
    case PageControlOperation::Lookup:
    case PageControlOperation::DrillDown:
    case PageControlOperation::AssistEdit:
      if (control->kind != ControlKind::Field) { Refuse(command.control, "not a field control"); }
      break;
    default: Refuse(command.control, "unsupported operation");
  }
  if (!page_.ControlVisible(control->name)) { Refuse(command.control, "control is hidden"); }
  if (command.operation != PageControlOperation::Read &&
      command.operation != PageControlOperation::ReadValue &&
      command.operation != PageControlOperation::Inspect &&
      command.operation != PageControlOperation::Filter && !page_.ControlEnabled(control->name)) {
    Refuse(command.control, "control is disabled");
  }
  if (WritesControl(command.operation) && !page_.ControlEditable(control->name)) {
    Refuse(command.control, "control is not editable");
  }
  return *control;
}

PageControlResult PageDispatcher::Execute(const PageControlCommand &command) {
  const ControlDef &control = RequireControl(command);
  switch (command.operation) {
    case PageControlOperation::Inspect:
      return {.caption = page_.ControlCaption(control.name),
              .enabled = static_cast<bool>(page_.ControlEnabled(control.name)),
              .editable = control.kind == ControlKind::Field &&
                          static_cast<bool>(page_.ControlEditable(control.name))};
    case PageControlOperation::ReadValue:
      return {.text = page_.ControlText(control.name),
              .ordinal = page_.ControlOrdinal(control.name),
              .value = page_.Control_Value(control.name),
              .caption = page_.ControlCaption(control.name),
              .enabled = static_cast<bool>(page_.ControlEnabled(control.name)),
              .editable = static_cast<bool>(page_.ControlEditable(control.name))};
    case PageControlOperation::Read:
      return {.text = page_.ControlText(control.name),
              .ordinal = page_.ControlOrdinal(control.name)};
    case PageControlOperation::Set: page_.SetControlText(control.name, command.text); break;
    case PageControlOperation::Filter: page_.SetControlFilter(control.name, command.text); break;
    case PageControlOperation::Action:
    case PageControlOperation::Lookup:
    case PageControlOperation::DrillDown:
    case PageControlOperation::AssistEdit:
      page_.RunControlTrigger(control.name, TriggerFor(command.operation));
      break;
    default: Refuse(command.control, "unsupported operation");
  }
  return {};
}

}
