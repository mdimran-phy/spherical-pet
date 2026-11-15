# Publish this repository

The intended public repository is `https://github.com/mdimran-phy/spherical-pet`.

The local source tree is independent of `eicdirc`. Build products, results, environment files and logs are ignored. Third-party sources and license notices are included, so a GitHub ZIP download works without submodules.

## GitHub CLI

Authenticate locally using GitHub CLI; do not put tokens in source files or chat:

```bash
gh auth login
```

If the repository does not yet exist:

```bash
gh repo create mdimran-phy/spherical-pet --public \
  --description "Geant4 spherical PET optical simulation with CLI, parameters, and event displays"
```

The prepared local Git repository uses `main` and the matching `origin`. Push it:

```bash
git push -u origin main
```

If Git HTTPS authentication is not yet configured, `gh auth setup-git` can configure GitHub CLI as the credential helper. Alternatively, use your existing SSH setup and an SSH remote.

## Browser and Git

Create a public repository named `spherical-pet` under `mdimran-phy`. Leave GitHub's README, .gitignore and license initialization unchecked because this source tree already supplies those files. Then run `git push -u origin main` from the prepared local repository.

## Source archive

GitHub’s **Code → Download ZIP** provides a source snapshot, including the checked-in course materials. GitHub's “Upload files” flow does not unpack a ZIP automatically; extract it and upload its contents, or use Git. Do not upload `build/`, simulation output, the old `pet_run_20261029` working directory, or authentication files.

After publishing, verify a fresh clone builds and `ctest --test-dir build --output-on-failure` passes in a configured Geant4 environment. A GitHub release may attach the source ZIP, but the repository itself should contain the actual source files.


## Current publication contents

The main README links the GAGG baseline, build/run instructions, native and offline displays, and the 112-slide beginner presentation. Curated course PDFs, PPTX, HTML, diagrams, source excerpts and small reference datasets are included. Local presentation ZIP bundles, generated slide previews, build products and raw simulation run directories remain ignored.

The GitHub repository was initialized separately with a one-line README. Preserve that initial commit when combining the histories: merge the remote history and resolve the README using the complete project documentation. Do not force-push over it.
