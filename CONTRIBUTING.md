# Contributing

When contributing to this repository, please first discuss the change you wish to make with the main developer `Luke de Munk` before making and pushing the change.

> Please note we have a [Code of Conduct](#code-of-conduct), please follow it in all your interactions with the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Questions](#questions)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Enhancements](#suggesting-enhancements)
- [Versions](#versions)
- [Merge Process](#merge-process)
- [Improving The Documentation](#improving-the-documentation)
- [Styleguides](#styleguides)

## Code of Conduct

### Our Pledge

In the interest of fostering an open and welcoming environment, we as contributors and maintainers pledge to making participation in our project and
our community a harassment-free experience for everyone, regardless of age, body size, disability, ethnicity, gender identity and expression, level of experience,
nationality, personal appearance, race, religion, or sexual identity and orientation.

### Our Standards

Examples of behavior that contributes to creating a positive environment include:

* Using welcoming and inclusive (English) language
* Being respectful of differing viewpoints and experiences
* Gracefully accepting constructive criticism

Examples of unacceptable behavior by participants include:

* The use of sexualized language or imagery and unwelcome sexual attention or advances
* Trolling, insulting/derogatory comments, and personal or political attacks
* Public or private harassment
* Publishing others' private information, such as a physical or electronic address, without explicit permission
* Other conduct which could reasonably be considered inappropriate in a professional setting

### Our Responsibilities

Project maintainers are responsible for clarifying the standards of acceptable behavior and are expected to take appropriate and fair corrective action in
response to any instances of unacceptable behavior.

Project maintainers have the right and responsibility to remove, edit, or reject comments, commits, code, wiki edits, issues, and other contributions
that are not aligned to this Code of Conduct, or to ban temporarily or permanently any contributor for other behaviors that they deem inappropriate,
threatening, offensive, or harmful.

### Scope

This Code of Conduct applies both within project spaces and in public spaces when an individual is representing the project or its community. Examples of
representing a project or community include using an official project e-mail address, posting via an official social media account, or acting as an appointed
representative at an online or offline event. Representation of a project may be further defined and clarified by project maintainers.

### Enforcement

Instances of abusive, harassing, or otherwise unacceptable behavior may be reported by contacting the project maintainer (Luke de Munk). All
complaints will be reviewed and investigated and will result in a response that is deemed necessary and appropriate to the circumstances. The project team is
obligated to maintain confidentiality with regard to the reporter of an incident. Further details of specific enforcement policies may be posted separately.

> Project maintainers who do not follow or enforce the Code of Conduct in good faith may face temporary or permanent repercussions as determined by other
members of the project's leadership.

### Attribution

This Code of Conduct is adapted from the [Contributor Covenant](http://contributor-covenant.org) version 1.4.

## Questions

> If you want to ask a question, we assume that you have read the available **documentation**.

Before you ask a question, it is best to search for existing [Issues](/issues) that might help you. In case you have found a suitable issue and still need clarification, you can write your question in this issue. It is also advisable to search the internet for answers first.

If you then still feel the need to ask a question and need clarification, we recommend the following:

- Open an [Issue](/issues/new).
- Provide as much context as you can about what you're running into.
- Provide project and platform versions (nodejs, npm, etc), depending on what seems relevant.

We will then take care of the issue as soon as possible.

## Reporting Bugs

### Before Submitting a Bug Report

A good bug report shouldn't leave others needing to chase you up for more information. Therefore, we ask you to investigate carefully, collect information and describe the issue in detail in your report. Please complete the following steps in advance to help us fix any potential bug as fast as possible.

- Make sure that you are using the latest version.
- Determine if your bug is really a bug and not an error on your side e.g. using incompatible environment components/versions (Make sure that you have read the **documentation**. If you are looking for support, you might want to check [this section](#i-have-a-question)).
- To see if other users have experienced (and potentially already solved) the same issue you are having, check if there is not already a bug report existing for your bug or error in the [bug tracker](issues?q=label%3Abug).
- Also make sure to search the internet (including Stack Overflow) to see if users outside of the GitHub community have discussed the issue.
- Collect information about the bug:
    - Stack trace (Traceback)
    - OS, Platform and Version (Windows, Linux, macOS, x86, ARM)
    - Version of the interpreter, compiler, SDK, runtime environment, package manager, depending on what seems relevant.
    - Possibly your input and the output
    - Can you reliably reproduce the issue? And can you also reproduce it with older versions?

### How Do I Submit a Good Bug Report?

> You must never report security related issues, vulnerabilities or bugs including sensitive information to the issue tracker, or elsewhere in public. Instead sensitive bugs must be sent by a private message to the main developer `Luke de Munk`.

We use GitHub issues to track bugs and errors. If you run into an issue with the project:

- Open an [Issue](/issues/new). (Since we can't be sure at this point whether it is a bug or not, we ask you not to talk about a bug yet and not to label the issue.)
- Explain the behavior you would expect and the actual behavior.
- Please provide as much context as possible and describe the *reproduction steps* that someone else can follow to recreate the issue on their own. This usually includes your code. For good bug reports you should isolate the problem and create a reduced test case.
- Provide the information you collected in the previous section.

Once it's filed:

- The project team will label the issue accordingly.
- A team member will try to reproduce the issue with your provided steps. If there are no reproduction steps or no obvious way to reproduce the issue, the team will ask you for those steps and mark the issue as `needs-repro`. Bugs with the `needs-repro` tag will not be addressed until they are reproduced.
- If the team is able to reproduce the issue, it will be marked `needs-fix`, as well as possibly other tags (such as `critical`), and the issue will be left to be fixed by someone.

## Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for this project, **including completely new features and minor improvements to existing functionality**. Following these guidelines will help maintainers to understand your suggestion and find related suggestions.

### Before Submitting an Enhancement

- Make sure that you are using the latest version.
- Read the documentation carefully and find out if the functionality is already covered, maybe by an individual configuration.
- Perform a [search](/issues) to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.
- Find out whether your idea fits with the scope and aims of the project. It's up to you to make a strong case to convince the project's developers of the merits of this feature. Keep in mind that we want features that will be useful to the majority of our users and not just a small subset. If you're just targeting a minority of users, consider writing an add-on/plugin library.

### How Do I Submit a Good Enhancement Suggestion?

Enhancement suggestions are tracked as [GitHub issues](/issues).

- Use a **clear and descriptive title** for the issue to identify the suggestion.
- Provide a **step-by-step description of the suggested enhancement** in as many details as possible.
- **Describe the current behavior** and **explain which behavior you expected to see instead** and why. At this point you can also tell which alternatives do not work for you.
- You may want to **include screenshots and animated GIFs** which help you demonstrate the steps or point out the part which the suggestion is related to. You can use [this tool](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux. 
- **Explain why this enhancement would be useful** to most project users. You may also want to point out the other projects that solved it better and which could serve as inspiration.

## Versions

The versioning scheme we use is an altered version of [Semantic Versioning](http://semver.org/). The
only difference is that (in the `MAJOR.MINOR.PATCH` format) the MINOR is NOT backwards compatible.

## Merge Process

1. NEVER push to the master branch and always use a new branch for updates.
2. Ensure any install or build dependencies are removed before the end of the layer when doing a build.
3. Ensure to remove any security related data or put it in the `.gitignore`.
4. Update the `CHANGELOG.md` with details of changes to the interface, this includes new environment variables, exposed ports, useful file locations and container parameters.
5. Increase the version numbers in all files and the `README.md` to the new version that this pull request would represent.
6. You may merge the pull request in once you have the sign-off of the main developer `Luke de Munk`.

## Improving The Documentation

When releasing a new version, ensure that the related documentation is updated as well. A list of documents that possibly need to be updated:

- `CHANGELOG.md`
- `README.md`
- Manuals
- When hardware changes are made:
    - Electrical diagram
    - Bill of Materials

## Style guides

In this section, only general styling conventions for this project are described.

> When coding something, look at the style guide (located in this repository) of the corresponding programming language ([HTML/CSS/JavaScript conventions](documentation/software/HTML_CSS_JAVASCRIPT_CONVENTIONS.md), [C/C++ conventions](documentation/software/CPP_CONVENTIONS.md))

### General Points of Interest

- Code and comments in English.
- Use full length variable names where possible.
- Use (O)bject (O)riented (P)rogramming where possible.
- Prevent magic numbers and magic strings.
- Use comment headers at the start of the file and methods.
- Use version locking when external libraries are added to the project.
- Credentials or vulnerable information should NEVER be in the project or repository. For example, use a credentials file and ignore it in `.gitignore`.