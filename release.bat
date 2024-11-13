git submodule update --remote --recursive
git add .
git commit -m "Update submodules to latest commits"
git tag %1
git push origin %1
