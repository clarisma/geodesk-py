@echo off
setlocal

:: Check if a tag is provided
if "%~1"=="" (
    echo Usage: %~0 <tag>
    exit /b 1
)

set "TAG=%~1"

:: Step 1: Update all submodules to their latest commit on the tracked branch
echo Updating submodules...
git submodule update --remote --recursive
if errorlevel 1 (
    echo Failed to update submodules.
    exit /b 1
)

:: Step 2: Commit the updated submodule references, if any
echo Committing submodule updates...
git add .
git commit -m "Update submodules to latest commits"
if errorlevel 1 (
    echo No changes to commit.
) else (
    git push
    if errorlevel 1 (
        echo Failed to push submodule updates.
        exit /b 1
    )
)

:: Step 3: Tag the repository
echo Tagging the repository with tag: %TAG%
git tag %TAG%
if errorlevel 1 (
    echo Failed to create tag.
    exit /b 1
)

:: Step 4: Push the tag to the remote repository
echo Pushing tag to remote...
git push origin %TAG%
if errorlevel 1 (
    echo Failed to push tag.
    exit /b 1
)

echo Tagging and submodule update completed successfully.
endlocal
