# This script releases a new version. Run it from the root
# directory of the repository for which a new version is being
# released. Version numbers use the format "{major}.{minor}.{patch}"

import os
import re
import subprocess
import sys

# Accepts e.g. "1.2.3", "v1.2.3", "1.2.3-rc4", "v1.2.3.rc4"
VERSION_PATTERN = re.compile(
    r"""^
    v?
    (?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)
    (?:[-.]rc(?P<rc>\d+))?
    $""",
    re.VERBOSE,
)

# Releases may only be made from this branch
RELEASE_BRANCH = "main"

RELEASE_NOTES_DIR = os.path.join("doc", "releases")

def _parse_version(version: str):
    """
    Returns the (major, minor, patch, rc) groups of `version`, where
    rc is None if `version` carries no release-candidate suffix, or
    raises a ValueError.
    """
    match = VERSION_PATTERN.match(version.strip())
    if match is None:
        raise ValueError(f"not a valid version number: {version!r}")
    return match.group("major", "minor", "patch", "rc")


def canonical_version(version: str):
    """
    Returns the version in the form "{major}.{minor}.{patch}"
    from `version` (which may begin with "v" and/or end with
    "-rc{number}" or ".rc{number}", or raises an error.
    """
    major, minor, patch, _ = _parse_version(version)
    return f"{major}.{minor}.{patch}"


def canonical_rc_version(version: str):
    """
    Returns the version in the form "{major}.{minor}.{patch}-rc{number}"
    from `version` (which may begin with "v" and must end with
    "-rc{number}" or ".rc{number}"
    """
    major, minor, patch, rc = _parse_version(version)
    if rc is None:
        raise ValueError(
            f"not a release-candidate version number: {version!r}"
        )
    return f"{major}.{minor}.{patch}-rc{rc}"


def replace_all(file: str, regex: str, new_string: str):
    """
    Replaces all occurrences of `regex` of `file` with `new_string`
    """
    with open(file, "r", encoding="utf-8") as f:
        contents = f.read()

    replaced = re.sub(regex, new_string, contents)
    if replaced == contents:
        return

    with open(file, "w", encoding="utf-8") as f:
        f.write(replaced)


def check_release_notes(version: str):
    """
    Checks for the presence of a file named "v{version}.md"
    in the "doc/releases" subdirectory; raises an error if
    the file does not exist.
    """
    path = os.path.join(
        RELEASE_NOTES_DIR, f"v{canonical_version(version)}.md"
    )
    if not os.path.isfile(path):
        raise FileNotFoundError(f"missing release notes: {path}")


def commit_and_push(version: str):
    """
    Stages all changes in the working tree (including untracked files)
    and commits them with the message "v{version}" (after converting
    the given `version` into its canonical form), then pushes the
    current branch. Does nothing if the working tree is clean.
    """
    message = f"v{canonical_version(version)}"
    if not _has_uncommitted_changes():
        print("Working tree is clean, nothing to commit")
        return
    check_branch()
    _run("git", "add", "--all")
    _run("git", "commit", "-m", message)
    _run("git", "push", "origin", RELEASE_BRANCH)


def tag_and_push(rc_version: str):
    """
    Creates the tag v{rc_version} (after converting the
    given`rc_version` into its canonical form and pushes
    the tag.
    """
    tag = f"v{canonical_rc_version(rc_version)}"
    _run("git", "tag", tag)
    _run("git", "push", "origin", tag)


def check_branch():
    """
    Raises an error unless the currently checked-out branch is the
    release branch.
    """
    branch = _current_branch()
    if branch != RELEASE_BRANCH:
        raise RuntimeError(
            f"releases must be made from {RELEASE_BRANCH!r}, "
            f"but {branch!r} is checked out"
        )


def release_candidate(rc_version: str):
    """
    Publishes the release candidate `rc_version`: checks the branch and
    the release notes, commits and pushes the working tree, then tags
    and pushes. The order matters, because pushing the tag starts the
    build, which runs against the commit the tag points to.
    """
    check_branch()
    check_release_notes(rc_version)
    commit_and_push(rc_version)
    tag_and_push(rc_version)


def _run(*command: str):
    """
    Echoes `command` and runs it, raising an error if it fails.
    """
    print("+ " + " ".join(command))
    subprocess.run(command, check=True)


def _capture(*command: str):
    """
    Runs `command` and returns its stripped standard output, raising an
    error if it fails.
    """
    result = subprocess.run(
        command, check=True, capture_output=True, text=True
    )
    return result.stdout.strip()


def _has_uncommitted_changes():
    """
    Returns True if the working tree contains modified, staged or
    untracked files.
    """
    return bool(_capture("git", "status", "--porcelain"))


def _current_branch():
    """
    Returns the name of the currently checked-out branch, or raises an
    error if HEAD is detached.
    """
    branch = _capture("git", "rev-parse", "--abbrev-ref", "HEAD")
    if branch == "HEAD":
        raise RuntimeError(
            "HEAD is detached; check out a branch before releasing"
        )
    return branch

SEMVER_MATCH = r'(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)'

version_arg = sys.argv[1]
version = canonical_version(version_arg)
replace_all("pyproject.toml",
    r'(?m)^[ \t]*version = "' + SEMVER_MATCH + '"[ \t]*$', version)
replace_all("src/version.h",
    r'(?m)^[ \t]*#define GEODESK_PY_VERSION "' + SEMVER_MATCH + '"[ \t]*$', version)
release_candidate(version_arg)

