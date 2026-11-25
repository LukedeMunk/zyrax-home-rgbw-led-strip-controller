# AGENTS_HTML_CSS_JAVASCRIPT.md

**Version 1.1**

## Purpose

This document defines the required practices for analyzing, creating, and modifying HTML, CSS, and JavaScript. It is language-specific and may be applied to any project.

Agents must prioritize:

- Semantic, accessible HTML.
- Predictable, maintainable CSS.
- Modular JavaScript with explicit ownership of state and DOM updates.
- Clear names, readable control flow, and focused responsibilities.
- Small changes that preserve compatible surrounding conventions.

Inspect the affected files and their callers before editing. Follow established local patterns when they do not conflict with this document or the explicit task.

## Sources of truth

Use the following precedence when instructions conflict:

1. Explicit task requirements.
2. The nearest applicable `AGENTS_HTML_CSS_JAVASCRIPT.md`.
3. `HTML_CSS_JAVASCRIPT_CONVENTIONS.md`.
4. Existing patterns in the affected project.
5. General HTML, CSS, and JavaScript conventions.

Do not perform broad formatting, renaming, restructuring, or modernization solely to make existing code match these rules.

## Change discipline

- Inspect related markup, styles, scripts, tests, and public interfaces before editing.
- Make the smallest coherent change that completely satisfies the task.
- Preserve unrelated user changes and existing public interfaces unless the task requires otherwise.
- Keep generated and vendored files unchanged unless the project explicitly requires regenerating them.
- Do not introduce a library, framework, build step, or abstraction without a clear need.
- Keep behavior, styling, state, and cleanup responsibilities with the component or module that owns them.

## Files and organization

Use `.html`, `.css`, and `.js` extensions. Do not use `.htm` for HTML files.

Organize authored JavaScript files in this order when applicable:

1. File header.
2. Imports.
3. Cached HTML element references.
4. Constants.
5. Module-level state.
6. Classes.
7. Main initialization or entry-point code.
8. Other functions.

Keep related definitions together and separate major sections with blank lines. Prefer ES modules with explicit imports and exports. Avoid wildcard-style or implicit global dependencies. Keep imports at module level unless a local import is needed to prevent a cycle or defer an optional dependency.

Keep HTML, CSS, data access, page coordination, and reusable component behavior in separate files or modules when they have distinct responsibilities. Do not create unnecessary files for short, cohesive code.

## Formatting

- Use four spaces for indentation. Do not use tabs for indentation.
- Add spaces around operators and after commas.
- Do not leave trailing whitespace.
- End JavaScript statements with semicolons.
- Put an opening brace on the declaration or control-flow line, preceded by one space.
- Put the matching closing brace on its own correctly indented line.
- Do not place multiple JavaScript statements on one line.
- Use parentheses for multiline expressions instead of fragile continuation techniques.
- Prefer wide lines and avoid unnecessary line breaks.
- Keep function calls, function definitions, `if` statements, collection literals, and other constructs on a single line when they do not exceed 100 characters.
- When a construct exceeds 100 characters, break it at a natural boundary, such as after a comma or before an operator.
- Indent continuation lines consistently and keep the resulting structure easy to scan.
- Do not split short expressions merely to make them visually symmetrical.
- Do not make code harder to understand merely to remain below 100 characters.
- The 100-character limit applies to authored source code. Long URLs, generated content, data strings, and other content that cannot reasonably be wrapped may exceed it.

### Blank lines and vertical spacing

Use blank lines to show structure and changes of responsibility. A reader should be able to identify
the phases of a function or component by scanning its vertical spacing. Do not remove all blank lines
from a non-trivial block, but do not place a blank line after every statement.

- Use exactly one blank line between major file sections, groups of unrelated declarations, class
  methods, and standalone functions.
- Inside a non-trivial function or method, use one blank line between distinct phases such as input
  validation, element lookup, state preparation, rendering or mutation, event registration, and the
  final result.
- Keep directly related statements together without blank lines. Examples include consecutive
  assignments to the same object, related element creation and configuration, adjacent append calls,
  and a short sequence that performs one operation.
- Keep consecutive guard clauses together when they form one validation phase. Add one blank line
  after that phase before the main operation.
- Add one blank line before a final `return` when it follows a multi-statement calculation or mutation
  phase. Do not add it in a one-line function, a short guard clause, or a branch whose result is
  already visually clear.
- Do not insert a blank line immediately after an opening brace or immediately before its matching
  closing brace.
- Do not separate `if` from `else`, `try` from `catch` or `finally`, or related branches with a blank
  line.
- Do not insert blank lines between every property of an object, item of an array, argument, CSS
  declaration, or closely related HTML child.
- Never use more than one consecutive blank line in authored HTML, CSS, or JavaScript.
- Blank lines must be empty and contain no spaces or tabs.
- Keep a documentation header attached to the declaration it documents. Do not place an additional
  blank line between the closing header separator and the function, method, or class declaration.

Use this JavaScript cadence for a method with multiple phases:

```javascript
render(configuration) {
    if (configuration == null) return;
    if (!configuration.id) throw new Error("A component ID is required.");

    const containerElem = document.getElementById(configuration.id);
    const titleElem = document.createElement("h2");

    titleElem.textContent = configuration.title;
    containerElem.replaceChildren(titleElem);

    this.containerElem = containerElem;
    this.configuration = configuration;

    return containerElem;
}
```

Avoid both a dense wall of unrelated operations and excessive spacing:

```javascript
// Too dense: lookup, creation, mutation, state, and return have no visual boundaries.
const containerElem = document.getElementById(configuration.id);
const titleElem = document.createElement("h2");
titleElem.textContent = configuration.title;
containerElem.replaceChildren(titleElem);
this.containerElem = containerElem;
return containerElem;

// Too sparse: every statement is separated even though the statements are closely related.
const iconElem = document.createElement("i");

iconElem.className = configuration.icon;

iconElem.setAttribute("aria-hidden", "true");
```

Apply the same principle per language:

- In HTML, use one blank line between major semantic regions such as the page header, main sections,
  and footer. Keep the children of one small component together.
- In CSS, use one blank line between rule sets, at-rules, or clearly separate component sections. Keep
  the declarations of one rule together; only split an unusually long declaration block when the
  groups have distinct responsibilities and the surrounding stylesheet follows that pattern.
- In JavaScript, use one blank line between class methods and between the logical phases of a
  non-trivial method, matching the balanced grouping demonstrated above.

## Naming

Use meaningful names and avoid uncommon abbreviations. Preserve established public names unless changing them is part of the task.

### HTML element IDs

Use `camelCase`, a maximum of 50 characters, and the applicable suffix:

| Element | Suffix | Example |
|---|---|---|
| Title | `Title` | `pageTitle` |
| Output field | `Field` | `successField` |
| Text input | `Txt` | `nameTxt` |
| Range input | `Range` | `ageRange` |
| Checkbox | `Cb` | `enableCb` |
| Select | `Select` | `audioSelect` |
| Button | `Btn` | `submitBtn` |

Use IDs only when an element needs a stable unique identifier, form association, fragment target, or JavaScript reference.

### CSS classes

Use `kebab-case`, a maximum of 50 characters, and no type prefix or suffix. Example: `notification-banner`.

### JavaScript

- Use `camelCase` for functions, methods, arguments, and variables.
- Use `#camelCase` for private class fields and methods.
- Use `PascalCase` for classes.
- Use `MACRO_CASE` for constants.
- Prefix fixed text constants with `TEXT_`, for example `TEXT_SAVE_SUCCESS`.
- Prefix functions that produce dynamic text with `VAR_TEXT_`, for example `VAR_TEXT_ITEMS_SELECTED`.
- Suffix variables containing HTML elements with `Elem`, for example `submitBtnElem`.
- Do not start identifiers with `$`.
- Prefer nouns or noun phrases for classes and values.
- Prefer verbs or verb phrases for functions and methods that perform actions.
- Give boolean values predicate names such as `isOpen`, `hasItems`, `canSubmit`, or `shouldRender`.

## Comments and headers

Comments must explain intent, constraints, assumptions, or non-obvious behavior. Do not repeat syntax already expressed clearly by the code.

- Keep comments accurate when behavior changes.
- Place comments immediately above the code they describe when practical.
- Use end-of-line comments only when the complete line remains readable and within 100 characters.
- Remove obsolete commented-out code.
- Use `TODO` only for a concrete remaining action.

Start every authored JavaScript file with this header, adapted to the file and project:

```javascript
/******************************************************************************/
/*
 * File:    example.js
 * Author:  Author name
 * Version: X.X.X
 *
 * Brief:   Short description of the file.
 *
 *          More information:
 *          https://example.com/project
 */
/******************************************************************************/
```

Document every authored JavaScript function and method, including constructors, private methods, async functions, callbacks with a distinct responsibility, and test functions. Use the established project header format. When no more specific format exists, use:

```javascript
/******************************************************************************/
/*!
 * @brief   Looks for a character in the specified keyword.
 * @param   keyword             String to search.
 * @param   character           Character to find.
 * @returns number              Index of the character, or -1 when not found.
 */
/******************************************************************************/
function lookForCharacter(keyword, character) {
    return keyword.indexOf(character);
}
```

Keep documentation synchronized with signatures, return values, side effects, and responsibilities. Omit `@returns` when a function has no meaningful return value.

## HTML

- Use valid, semantic HTML elements before generic `div` or `span` elements.
- Keep every HTML tag, including all attributes and the closing `>`, on one physical line,
  regardless of length. Never insert a newline between a tag's opening `<` and closing `>`.
- Place each nested child or sibling element on its own physical line and indent it according to
  the document hierarchy. Do not place multiple nested elements on the same physical line.
- An element with no content may keep its opening and closing tags together, such as
  `<h2 data-i18n="pageTitle"></h2>`.

Incorrect:

```html
<article class="card"><h2 data-i18n="cardTitle"></h2><p data-i18n="cardDescription"></p><div class="card-actions"><a href="/download" data-i18n="download"></a></div></article>
```

Correct:

```html
<article class="card">
    <h2 data-i18n="cardTitle"></h2>
    <p data-i18n="cardDescription"></p>
    <div class="card-actions">
        <a href="/download" data-i18n="download"></a>
    </div>
</article>
```

- Keep the document hierarchy logical and use headings in a meaningful order.
- Use controls that match the action: links for navigation and buttons for actions.
- Associate every form control with a visible label unless an accessible name is deliberately supplied another way.
- Keep IDs unique and use the naming rules in this document.
- Order attributes as `id`, `class`, other attributes, and `style`.
- Avoid inline styles when the rule belongs in reusable CSS.
- Load classic external scripts with `<script src="script.js"></script>`. Use `type="module"` when loading an ES module.
- Keep markup declarative. Do not duplicate content or state in HTML and JavaScript without a defined source of truth.

## UI text and language packages

- Put all user-visible UI text in an available language package, such as `language_pack_en.js` or `language-package-dutch.js`; do not hardcode that text in HTML or implementation JavaScript.
- Use `TEXT_` constants for fixed UI text.
- Use `VAR_TEXT_` functions for dynamic UI text that interpolates values, selects variants, or otherwise constructs a message at runtime.
- Reuse the project's existing language package, naming, import, and export patterns when they are available.
- Text that is not visible to users, such as internal identifiers and diagnostic-only developer messages, does not need to be placed in a language package.

## Accessibility and ARIA

- Prefer native semantic elements and behavior over custom ARIA implementations.
- Add ARIA only when native semantics do not express the required role, name, state, or relationship.
- Ensure interactive elements are keyboard-operable and have visible focus states.
- Preserve logical focus order. Move or restore focus deliberately when opening and closing overlays, menus, or dialogs.
- Give icon-only controls an accessible name.
- Communicate dynamic status and validation feedback accessibly when the user must be informed of it.
- Keep ARIA state synchronized with visible state, including `aria-expanded`, `aria-selected`, `aria-invalid`, and `aria-hidden` where applicable.
- Provide useful alternative text for informative images and empty alternative text for decorative images.
- Do not use color as the only way to communicate meaning.

## CSS

- Use `kebab-case` class names and keep selectors tied to the component or responsibility they style.
- Prefer classes over IDs for styling.
- Keep selector specificity low and avoid `!important` unless an existing cascade contract requires it.
- Do not use overly broad selectors when they can unintentionally style unrelated components.
- Keep shared tokens, reusable primitives, component styles, and page-specific overrides clearly separated.
- Reuse variables for repeated colors, spacing, sizes, layers, and transition values instead of duplicating magic values.
- Keep declarations consistently ordered according to the surrounding stylesheet.
- Define complete interaction states where relevant: default, hover, focus, active, disabled, loading, success, warning, and error.
- Design responsive behavior intentionally. Test affected layouts at narrow and wide viewport sizes.
- Prefer layout systems such as flexbox and grid over position-dependent spacing hacks.
- Scope animations and transitions to the properties that actually change, and provide stable initial and final states.
- Respect reduced-motion preferences when motion is not essential to understanding the interface.

## JavaScript structure

Use `const` by default and `let` only when reassignment is required. Do not use `var` in new code.

- Keep functions focused on one responsibility.
- Validate inputs before performing the main operation.
- Prefer early returns when they reduce nesting.
- Keep the normal execution path easy to follow.
- Return consistent types across execution paths.
- Extract a helper when a block has a separate responsibility or a meaningful reusable name.
- Do not split short, cohesive functions into unnecessary helpers.
- Avoid hidden mutation of module-level or global state.
- Prefer constants over repeated magic values.
- Keep constructors limited to explicit initialization; do not perform unrelated or surprising work in them.
- Use classes only when state and behavior belong together. Prefer focused functions when no shared instance state exists.
- Organize classes with constants and fields first, followed by the constructor, public methods, and private methods.

## State, DOM, and components

- Give each piece of mutable state one clear owner.
- Derive display values from state instead of maintaining unsynchronized copies.
- Cache repeatedly used HTML element references with `const` and an `Elem` suffix.
- Query within the narrowest responsible root instead of repeatedly searching the complete document.
- Keep direct DOM manipulation inside the component or module that owns the affected markup.
- Prefer `textContent`, element creation, and explicit attributes over HTML-string construction.
- Make render, update, show, hide, and destroy behavior predictable and idempotent where practical.
- Do not replace or recreate an entire component when a localized state or validation update is sufficient.
- Keep page modules responsible for composition and coordination, not component-internal rendering details.

## Events and lifecycle cleanup

- Keep event ownership explicit and use named handlers when they must later be removed.
- Prevent duplicate listeners when a component rerenders or remounts.
- Remove listeners, observers, timers, animation frames, subscriptions, and temporary DOM elements during cleanup.
- Make delayed callbacks cancellable and prevent stale callbacks from mutating destroyed or replaced components.
- Handle rapid open, close, rerender, and destroy sequences without leaving stale state.
- Use event delegation when it simplifies ownership without obscuring behavior.
- Use custom events or callbacks to report component actions instead of tightly coupling components to page functions.

## Error handling

- Validate boundary data, configuration, and required elements before using them.
- Fail with a clear error when a required invariant is missing.
- Do not silently swallow exceptions.
- Handle rejected promises and unsuccessful network responses deliberately.
- Keep user-facing error presentation separate from transport and domain error handling.
- Clean up transient UI state in failure paths, including loading states, overlays, timers, and scroll locks.

## Security and dynamic content

- Treat external, user-provided, stored, and server-provided content as untrusted.
- Insert plain text with `textContent` rather than `innerHTML`.
- Do not interpolate untrusted content into HTML, CSS, selectors, script, event-handler attributes, or URLs.
- Sanitize dynamic HTML with an established project mechanism when HTML insertion is genuinely required.
- Validate URLs and restrict unexpected protocols before assigning navigation or resource attributes.
- Do not expose secrets, credentials, tokens, or sensitive configuration in client-side source code.
- Avoid inline script and event-handler attributes when event listeners can be registered from JavaScript.

## Testing and validation

- Add or update focused tests for changed behavior and likely regressions.
- Test normal behavior, invalid input, empty states, error paths, and lifecycle cleanup where relevant.
- Test interactive components with keyboard as well as pointer behavior.
- Verify rapid repeated actions and destroy or rerender paths for components that use timers or asynchronous work.
- Validate HTML structure, JavaScript syntax, module boundaries, and local asset references with the project's existing tools.
- Check responsive layouts and affected interaction states at representative narrow and wide viewport sizes.
- Regenerate derived assets only through their documented build process and verify that generated output is current.
- Run the narrowest relevant checks while developing, then run the complete applicable test and validation suite before completion.

## Completion checklist

Before considering a change complete, confirm that:

- The change follows the nearest applicable instructions and established compatible patterns.
- Names follow the HTML, CSS, and JavaScript conventions in this document.
- Indentation uses four spaces and no tabs.
- Authored lines remain on one line when readable and no longer than 100 characters.
- Longer constructs wrap at natural boundaries with consistent continuation indentation.
- Blank lines separate logical phases without splitting cohesive operations or creating repeated empty
  lines.
- New and modified JavaScript files and functions have accurate required headers.
- HTML is semantic, accessible, and uses correctly associated controls and labels.
- Every HTML tag remains on one physical line, with no newline between its opening `<` and
  closing `>`, and every nested child or sibling element uses its own correctly indented line.
- All user-visible UI text uses the available language package, with `TEXT_` constants for fixed text and `VAR_TEXT_` functions for dynamic text.
- CSS is scoped, responsive, and does not introduce unnecessary specificity.
- JavaScript state, DOM updates, events, and cleanup have clear owners.
- Dynamic content is handled safely.
- Relevant tests and validation checks pass.
- No unrelated files, interfaces, or formatting were changed.
