#pragma once

#include "meta/Ids.h"

#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief The static declaration of an AL page: its type, its source table and its CONTROL TREE.

namespace agiru {

/// \brief AL's `PageType` values, `devenv-pagetype-property.md`, in the order the page lists them.
///
/// \note THE CATEGORY IS NOT THE VALUE. `devenv-page-types-and-layouts.md` divides the nineteen
///       into entity-oriented and collection-oriented, and an action on a collection page acts on
///       the SELECTED ROWS while the same action on a card acts on the entity the title names.
///       `EntityOriented()` answers that, so a renderer never spells the division itself.
/// \warning `Api` IS SPELLED AS `ClientType` SPELLS IT AND NOT AS THE PROPERTY PAGE DOES. The
///          property page writes `API`; the door's spelling table is case-folded across all of
///          `include/`, so two spellings of one word blind it for every AL body that names either
///          -- and `ClientType::Api` is the documentation's own spelling of the same word, used
///          44 times in the BaseApp. A `PageType` value is a property VALUE the generator matches
///          case-insensitively and never an identifier an AL body writes, so the deviation costs
///          the reader nothing and keeps `ClientType::Api` resolvable.
enum class PageType : std::uint8_t {
  Card,
  List,
  RoleCenter,
  CardPart,
  ListPart,
  Document,
  Worksheet,
  ListPlus,
  ConfirmationDialog,
  NavigatePage,
  StandardDialog,
  Api,
  ReportPreview,
  ReportProcessingOnly,
  XmlPort,
  HeadlinePart,
  PromptDialog,
  ConfigurationDialog,
  UserControlHost,
};

/// \brief Whether a page shows ONE entity rather than a set of rows.
///
/// \param type The page's type.
/// \return True for the entity-oriented types.
///
/// `devenv-page-types-and-layouts.md` names `Card`, `Document`, `ListPlus` and `CardPart` as
/// entity-oriented and `List`, `Worksheet` and `ListPart` as collection-oriented; the dialog types
/// are "suited for both" and count as entity-oriented, since a dialog carries no row set.
[[nodiscard]] constexpr bool EntityOriented(PageType type) {
  return type != PageType::List && type != PageType::Worksheet && type != PageType::ListPart &&
         type != PageType::RoleCenter && type != PageType::Api;
}

/// \brief What a control IS, which AL writes as the word that opens the control.
///
/// \warning A KIND THE GENERATOR DOES NOT KNOW IS A TRANSLATION ERROR AND NEVER A DROP. The
///          predecessor of this enum was three predicates over lowercased strings, and every
///          container kind matched none of them -- 59 979 controls visited for their children and
///          then discarded (board:0553).
enum class ControlKind : std::uint8_t {
  Area,             ///< `area(Content)` and its fourteen siblings; see AreaKind.
  Group,            ///< `group(...)`, which a FastTab is (board:0561).
  Repeater,         ///< `repeater(...)`, the row band of a list.
  CueGroup,         ///< `cuegroup(...)`, a role centre's tiles.
  Grid,             ///< `grid(...)`.
  Fixed,            ///< `fixed(...)`, the deprecated fixed layout.
  Field,            ///< `field(Name; Expression)`.
  Label,            ///< `label(...)`, a caption with no source.
  Part,             ///< `part(Name; Page)`, an embedded page.
  SystemPart,       ///< `systempart(Name; Notes)`, a platform-provided part.
  ChartPart,        ///< `chartpart(...)`.
  UserControl,      ///< `usercontrol(Name; AddIn)` (board:0479).
  View,             ///< `view(Name)`, a named filter and sort on a list (board:0542).
  Action,           ///< `action(Name)`.
  ActionRef,        ///< `actionref(Name; Action)` (board:0555).
  Separator,        ///< `separator(Name)`, which may be a header (board:0478).
  FileUploadAction, ///< `fileuploadaction(Name)` (board:0300).
  SystemAction,     ///< `systemaction(Name)`.

  /// \brief A nested `actions` block, which a `cuegroup` carries so a cue can be drilled into.
  Actions,

  /// \brief A control word the generator does not know, with the word itself in `source`.
  ///
  /// \warning IT IS A COUNTED HOLE AND NOT A DROP. board:0553's rule is that an unknown kind must
  ///          not vanish; this keeps the node, its children and the AL word, so the population is
  ///          a `grep` over the generated tree rather than a guess over the AL source.
  Unknown,
};

/// \brief Whether a control holds other controls.
/// \param kind The control's kind.
/// \return True when the control's children are its layout.
[[nodiscard]] constexpr bool Container(ControlKind kind) {
  return kind == ControlKind::Area || kind == ControlKind::Group || kind == ControlKind::Repeater ||
         kind == ControlKind::CueGroup || kind == ControlKind::Grid || kind == ControlKind::Fixed ||
         kind == ControlKind::Actions;
}

/// \brief The argument of `area(...)`, which decides WHERE a control tree hangs.
///
/// \note THE FIFTEEN ARE MEASURED AND NOT INVENTED. `~/Git/BCApps/src`, 2026-09-04: `content`
///       8 383, `processing` 2 977, `promoted` 2 300, `factboxes` 2 103, `navigation` 1 910,
///       `reporting` 488, `creation` 216, `sections` 204, `rolecenter` 162, `embedding` 145,
///       `systemactions` 13, `prompt` 7, `promptoptions` 5, `prompting` 5, `promptguide` 2.
enum class AreaKind : std::uint8_t {
  None, ///< The control is not an area.
  Content,
  FactBoxes,
  Sections,
  RoleCenter,
  Embedding,
  Processing,
  Navigation,
  Reporting,
  Creation,
  Promoted,
  SystemActions,
  Prompt,
  PromptOptions,
  Prompting,
  PromptGuide,
};

/// \brief One control's declaration, as static const data.
///
/// Every `ControlDef` the generator writes is part of a `constexpr` tree in `.rodata`: the renderer
/// walks the tree the way the AL source is written, and the SIBLING ORDER is part of the meaning --
/// `devenv-page-types-and-layouts.md` sizes "the LAST part on the page" and "the FIRST ListPart",
/// which no flat list with a depth column can answer without rebuilding the tree (board:0553).
struct ControlDef {
  ControlKind kind{};      ///< What the control is.
  std::string_view name{}; ///< The AL name, spaces and all; empty on an anonymous container.

  /// \brief The `area(...)` argument, `AreaKind::None` on anything that is not an area.
  AreaKind area = AreaKind::None;

  /// \brief The `Caption` property, or the name where AL declares none.
  std::string_view caption{};

  /// \brief The AL expression the control shows, exactly as AL wrote it: `Rec."No."`, `Amount`.
  ///
  /// \note IT IS THE EXPRESSION AND NOT A FIELD NUMBER. A page field's source may be a record
  ///       field, a page variable or an expression, and only the first has a number -- so the text
  ///       is what every kind has in common and `field` carries the number when there is one.
  std::string_view source{};

  /// \brief The source table's field number when the control shows one, 0 otherwise.
  FieldNo field{};

  /// \brief The page a `part` embeds, or the action's `RunObject` page; 0 when there is none.
  PageId page{};

  /// \brief The `ToolTip` property (board:0385).
  std::string_view toolTip{};

  /// \brief The `ApplicationArea` property, as AL wrote it: the comma list of areas (board:0399).
  std::string_view applicationArea{};

  /// \brief The `Visible`, `Enabled` and `Editable` properties, as AL wrote them.
  ///
  /// \warning THEY ARE AL EXPRESSIONS AND NOT FLAGS. 2 052 `Enabled` declarations name a page
  ///          variable far more often than they say `false`, so a `bool` here would be a lie for
  ///          most of them; the renderer evaluates what the page declares (board:0400, board:0401,
  ///          board:0402).
  std::string_view visible{};
  std::string_view enabled{};  ///< \see visible
  std::string_view editable{}; ///< \see visible

  /// \brief The `Importance` property, as AL wrote it: what a FastTab shows before Show more
  ///        (board:0411).
  std::string_view importance{};

  /// \brief The `Style` and `StyleExpr` properties, which colour a control together (board:0415).
  std::string_view style{};
  std::string_view styleExpr{}; ///< \see style

  /// \brief The `Image` property: the icon the client draws (board:0416).
  std::string_view image{};

  /// \brief The `ShortcutKey` property (board:0417).
  std::string_view shortcutKey{};

  /// \brief The `RunObject`, `RunPageLink`, `RunPageView` and `RunPageMode` properties, as AL wrote
  ///        them: what an action runs and how it is filtered (board:0433).
  std::string_view runObject{};
  std::string_view runPageLink{}; ///< \see runObject
  std::string_view runPageView{}; ///< \see runObject
  std::string_view runPageMode{}; ///< \see runObject

  /// \brief The `SubPageLink` and `SubPageView` properties of an embedded part (board:0430).
  std::string_view subPageLink{};
  std::string_view subPageView{}; ///< \see subPageLink

  /// \brief The `Multiplicity` property on a page part: how many of the part an API exposes.
  std::string_view multiplicity{};

  /// \brief The `Filters` property on a `view`, as AL wrote it: `where(Field = const(x))`.
  std::string_view filters{};

  /// \brief The `OrderBy` property on a `view`, as AL wrote it: `ascending(Field)`.
  ///
  /// \note A VIEW TAKES ONE DIRECTION AND A QUERY TAKES A LIST, which is board:0352's finding --
  ///       so this is carried as AL wrote it and read by whoever applies it.
  std::string_view orderBy{};

  /// \brief The `ODataEDMType` property on a page field: the EDM type an API publishes it as.
  std::string_view odataEdmType{};

  /// \brief The `ShowCaption` property: the control renders without its label (board:0395).
  bool showCaption = true;

  /// \brief The `Ellipsis` property: the action's caption ends in `...` (board:0418).
  bool ellipsis = false;

  /// \brief The `MultiLine` property: the control is a text area (board:0408).
  bool multiLine = false;

  /// \brief The `QuickEntry` property: the control is on the Enter key's path (board:0406).
  bool quickEntry = true;

  /// \brief The `ShowMandatory` property, which marks and enforces nothing (board:0407).
  bool showMandatory = false;

  /// \brief The `HideValue` property: the value is blanked and the control stays (board:0409).
  std::string_view hideValue{};

  /// \brief The `Width` property: a suggestion in characters, 0 when none is declared
  ///        (board:0423).
  std::uint16_t width = 0;

  /// \brief The `FreezeColumn` property: the control the left of a wide list ends at (board:0420).
  std::string_view freezeColumn{};

  /// \brief The `Lookup`, `DrillDown` and `AssistEdit` properties, as AL wrote them (board:0336,
  ///        board:0337, board:0474).
  std::string_view lookup{};
  std::string_view drillDown{};  ///< \see lookup
  std::string_view assistEdit{}; ///< \see lookup

  /// \brief The `ExtendedDataType` property, as AL wrote it: `EMail`, `URL`, `RichContent`, ...
  ///        A control may override what its field declares (board:0329).
  std::string_view extendedDataType{};

  /// \brief The `InstructionalText` property, which stands at the top of a group (board:0387).
  std::string_view instructionalText{};

  /// \brief The `LookupPageId` and `DrillDownPageId` properties on the CONTROL, which override
  ///        what the field declares (board:0334, board:0335).
  PageId lookupPageId{};
  PageId drillDownPageId{}; ///< \see lookupPageId

  /// \brief The `UpdatePropagation` property, as AL wrote it: whether a part's change reaches its
  ///        host page.
  std::string_view updatePropagation{};

  /// \brief The `OptionCaption` property, as AL wrote it: the comma list a control shows for an
  ///        option field (board:0053).
  std::string_view optionCaption{};

  /// \brief The `AboutTitle` and `AboutText` properties: the teaching tip, which needs BOTH or it
  ///        does not appear (board:0388).
  std::string_view aboutTitle{};
  std::string_view aboutText{}; ///< \see aboutTitle

  /// \brief The `IndentationColumn` and `IndentationControls` properties, as AL wrote them: an
  ///        indented list is four properties that only work together (board:0419).
  std::string_view indentationColumn{};
  std::string_view indentationControls{}; ///< \see indentationColumn

  /// \brief The `ShowAs` property of an action: what the action bar renders it as (board:0487).
  std::string_view showAs{};

  /// \brief The `InFooterBar` property: the action sits in the footer bar (board:0425).
  bool inFooterBar = false;

  /// \brief The `RunPageOnRec` property: the run object opens on the CURRENT record rather than
  ///        on a new one (board:0433).
  bool runPageOnRec = false;

  /// \brief The `ShowFilter` property on a part, which a part cannot get back (board:0421).
  bool showFilter = true;

  /// \brief The `NotBlank` property on the CONTROL: the client refuses an empty entry
  ///        (board:0319).
  bool notBlank = false;

  /// \brief The `ShowAsTree` property: a repeater renders as an expandable tree (board:0560).
  bool showAsTree = false;

  /// \brief The `IsHeader` property of a separator, which makes it a heading (board:0478).
  bool isHeader = false;

  /// \brief The `Provider` property, as AL wrote it: the part an action's data comes from.
  std::string_view provider{};

  /// \brief The `MinValue`, `MaxValue`, `BlankNumbers` and `MaskType` properties, as AL wrote
  ///        them: a control may override what its field declares (board:0317, board:0318,
  ///        board:0323, board:0330).
  std::string_view minValue{};
  std::string_view maxValue{};     ///< \see minValue
  std::string_view blankNumbers{}; ///< \see minValue
  std::string_view maskType{};     ///< \see minValue

  /// \brief The `ColumnSpan` and `RowSpan` properties: how many grid cells a control fills; the
  ///        web client ignores `RowSpan` (board:0422). 0 when none is declared.
  std::uint16_t columnSpan = 0;
  std::uint16_t rowSpan = 0; ///< \see columnSpan

  /// \brief The `ClosingDates` and `Numeric` properties on the CONTROL (board:0326, board:0320).
  bool closingDates = false;
  bool numeric = false; ///< \see closingDates

  /// \brief The `ValuesAllowed` property, as AL wrote it (board:0322).
  std::string_view valuesAllowed{};

  /// \brief The `TreeInitialState` property, as AL wrote it: how an expandable list opens
  ///        (board:0560).
  std::string_view treeInitialState{};

  /// \brief The `CueGroupLayout` property, as AL wrote it: whether a cue group is wide
  ///        (board:0428).
  std::string_view cueGroupLayout{};

  /// \brief The `AllowMultipleFiles` and `AllowedFileExtensions` properties of a file-upload
  ///        action, as AL wrote them (board:0300).
  bool allowMultipleFiles = false;
  std::string_view allowedFileExtensions{}; ///< \see allowMultipleFiles

  /// \brief The `EntityName` and `EntitySetName` properties on a CONTROL, which an API page's
  ///        parts carry (board:0567).
  std::string_view entityName{};
  std::string_view entitySetName{}; ///< \see entityName

  /// \brief The `Description` property, which the compiler carries and nothing reads.
  std::string_view description{};

  /// \brief The `GridLayout` property, as AL wrote it: rows or columns (board:0422).
  std::string_view gridLayout{};

  /// \brief The `Gesture` property, as AL wrote it: the touch gesture that runs the action
  ///        (board:0426).
  std::string_view gesture{};

  /// \brief The `FlowTemplateCategoryName` property, as AL wrote it: the Power Automate category
  ///        a template action belongs to.
  std::string_view flowTemplateCategoryName{};

  /// \brief The `AutoFormatType` and `AutoFormatExpression` properties, as AL wrote them: a
  ///        control may override what its field says (board:0437).
  std::string_view autoFormatType{};
  std::string_view autoFormatExpression{}; ///< \see autoFormatType

  /// \brief The `CaptionClass` property, as AL wrote it (board:0384).
  std::string_view captionClass{};

  /// \brief The `DecimalPlaces` property, as AL wrote it: `2` or `2:5` (board:0325).
  std::string_view decimalPlaces{};

  /// \brief The `TableRelation` property, as AL wrote it: a control may declare its own
  ///        (board:0331).
  std::string_view tableRelation{};

  /// \brief The `BlankZero` property: a zero renders as nothing (board:0324).
  bool blankZero = false;

  /// \brief The `Scope` property of an action: `Repeater` puts it on the row (board:0362).
  std::string_view scope{};

  /// \brief The `AccessByPermission` property: the element is gone for a user without it
  ///        (board:0377).
  std::string_view accessByPermission{};

  /// \brief The `ObsoleteState` property, as AL wrote it.
  std::string_view obsoleteState{};

  /// \brief The controls this one holds, in SOURCE ORDER, empty on a leaf.
  std::span<const ControlDef> children{};
};

/// \brief One page's declaration.
struct PageDef {
  PageId id{};                ///< The AL page number.
  std::string_view name{};    ///< The AL name: `"Customer Card"`.
  std::string_view caption{}; ///< The `Caption` property, which is the name where none is declared.
  PageType type = PageType::Card; ///< The `PageType` property (board:0429).

  /// \brief The `SourceTable` property's table number, 0 when the page names none.
  TableId source{};

  /// \brief The `SourceTableView` property, as AL wrote it (board:0432).
  std::string_view sourceTableView{};

  /// \brief The `layout` section, which is areas at the top level.
  std::span<const ControlDef> layout{};

  /// \brief The `actions` section, which is areas at the top level.
  std::span<const ControlDef> actions{};

  /// \brief The `views` section: named filters over the source, each a control of kind `View`.
  ///
  /// \note IT WAS PARSED AS NOTHING AND SKIPPED, which is what an identifier followed by a brace
  ///       does in the page body -- so 179 `Filters` and 113 `OrderBy` declarations went nowhere
  ///       and nothing said so (board:0623). A view carries its `Caption`, its `Filters` and its
  ///       `OrderBy` as properties, so it IS a control and needs no shape of its own.
  std::span<const ControlDef> views{};

  /// \brief The `Editable`, `InsertAllowed`, `ModifyAllowed` and `DeleteAllowed` properties, as AL
  ///        wrote them (board:0403).
  std::string_view editable{};
  std::string_view insertAllowed{}; ///< \see editable
  std::string_view modifyAllowed{}; ///< \see editable
  std::string_view deleteAllowed{}; ///< \see editable

  /// \brief The `DelayedInsert` property (board:0404).
  bool delayedInsert = false;

  /// \brief The `LinksAllowed` property (board:0405).
  bool linksAllowed = true;

  /// \brief The `ShowFilter` property (board:0421).
  bool showFilter = true;

  /// \brief The `RefreshOnActivate` property (board:0414).
  bool refreshOnActivate = false;

  /// \brief The `SaveValues` property (board:0413).
  bool saveValues = false;

  /// \brief The `AnalysisModeEnabled` property (board:0460).
  bool analysisModeEnabled = true;

  /// \brief The `CardPageId` property: the card a list opens a row in (board:0434).
  PageId cardPageId{};

  /// \brief The `DataCaptionExpression` and `DataCaptionFields` properties, as AL wrote them
  ///        (board:0374, board:0375).
  std::string_view dataCaptionExpression{};
  std::string_view dataCaptionFields{}; ///< \see dataCaptionExpression

  /// \brief The `ApplicationArea` and `UsageCategory` properties: whether Tell Me finds the page
  ///        (board:0466).
  std::string_view applicationArea{};
  std::string_view usageCategory{}; ///< \see applicationArea

  /// \brief The `AdditionalSearchTerms` property (board:0389).
  std::string_view additionalSearchTerms{};

  /// \brief The `InstructionalText` property, which stands at the top of the page (board:0387).
  std::string_view instructionalText{};

  /// \brief The `PromotedActionCategories` property, as AL wrote it (board:0477).
  std::string_view promotedActionCategories{};

  /// \brief The `Permissions` property: what the page may reach beyond its caller (board:0376).
  std::string_view permissions{};

  /// \brief The `SourceTableTemporary` property: the page's own source is a temporary record
  ///        (board:0449).
  bool sourceTableTemporary = false;

  /// \brief The `AutoSplitKey` property: a new line goes between two existing ones (board:0354).
  bool autoSplitKey = false;

  /// \brief The `AboutTitle` and `AboutText` properties on the PAGE (board:0388).
  std::string_view aboutTitle{};
  std::string_view aboutText{}; ///< \see aboutTitle

  /// \brief The `PopulateAllFields` property: the page reads every field of its source rather
  ///        than the ones its controls name.
  bool populateAllFields = false;

  /// \brief The `InherentPermissions` and `InherentEntitlements` properties on the PAGE
  ///        (board:0378).
  std::string_view inherentPermissions{};
  std::string_view inherentEntitlements{}; ///< \see inherentPermissions

  /// \brief The `AccessByPermission` property on the PAGE (board:0377).
  std::string_view accessByPermission{};

  /// \brief The `QueryCategory` property, as AL wrote it: where an API page is listed
  ///        (board:0464).
  std::string_view queryCategory{};

  /// \brief The `ODataKeyFields` property, as AL wrote it: the fields an OData key is built from
  ///        (board:0390).
  std::string_view odataKeyFields{};

  /// \brief The `ContextSensitiveHelpPage` property, as AL wrote it (board:0393).
  std::string_view contextSensitiveHelpPage{};

  /// \brief The API identity: `APIPublisher`, `APIGroup`, `APIVersion`, `EntityName`,
  ///        `EntitySetName`, `EntityCaption` and `EntitySetCaption`, as AL wrote them. An API
  ///        object is addressed by publisher, group and version (board:0465, board:0390).
  std::string_view apiPublisher{};
  std::string_view apiGroup{};         ///< \see apiPublisher
  std::string_view apiVersion{};       ///< \see apiPublisher
  std::string_view entityName{};       ///< \see apiPublisher
  std::string_view entitySetName{};    ///< \see apiPublisher
  std::string_view entityCaption{};    ///< \see apiPublisher
  std::string_view entitySetCaption{}; ///< \see apiPublisher

  /// \brief The `ChangeTrackingAllowed` property: whether the API exposes a delta link.
  bool changeTrackingAllowed = false;

  /// \brief The `IsPreview` property: the object is not yet supported for production.
  bool isPreview = false;

  /// \brief The `DataAccessIntent` property on the PAGE, as AL wrote it.
  std::string_view dataAccessIntent{};

  /// \brief The `HelpLink` property, as AL wrote it (board:0393).
  std::string_view helpLink{};

  /// \brief The `Description` property, which the compiler carries and nothing reads.
  std::string_view description{};

  /// \brief The `Extensible`, `Access` and `ObsoleteState` properties, as AL wrote them.
  std::string_view extensible{};
  std::string_view access{};        ///< \see extensible
  std::string_view obsoleteState{}; ///< \see extensible
};

/// \brief Finds a control by its AL name, anywhere in a tree.
///
/// \param controls The controls to walk.
/// \param name     The AL name, compared exactly.
/// \return The control, or `nullptr` when the tree names no such control.
///
/// \note A WALK AND NOT A MAP. A page's whole tree is some tens of controls, this is reached from a
///       test and from a client action rather than from a posting loop, and a map would cost a
///       relocation per page in every process.
[[nodiscard]] constexpr const ControlDef *Control(std::span<const ControlDef> controls,
                                                  std::string_view name) {
  for (const ControlDef &control : controls) {
    if (control.name == name) { return &control; }
    if (const ControlDef *found = Control(control.children, name); found != nullptr) {
      return found;
    }
  }
  return nullptr;
}

/// \brief How deep a control tree goes, which is what proves the nesting survived.
///
/// \param controls The controls to walk.
/// \return 0 for an empty tree, 1 for a flat one, and one more per level below that.
///
/// \warning IT EXISTS FOR A `static_assert` AND NOT FOR THE RENDERER. board:0553's negative control
///          is that flattening the tree must FAIL TO COMPILE, and a check over the control NAMES
///          stays green under flattening -- so the depth is what the generated assertion states.
[[nodiscard]] constexpr std::size_t Depth(std::span<const ControlDef> controls) {
  std::size_t deepest = 0;
  for (const ControlDef &control : controls) {
    const std::size_t below = Depth(control.children);
    if (below + 1 > deepest) { deepest = below + 1; }
  }
  return deepest;
}

}
