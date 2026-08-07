# Flatpak Build

The manifest builds Acid Cam from this checkout and a pinned `libacidcam`
GitHub revision, so a sibling `libacidcam` checkout is not required. Install
the KDE 6.10 SDK and runtime, then run these commands from `Acid.Cam.v2.Qt`:

```sh
flatpak install --user flathub org.kde.Platform//6.10 org.kde.Sdk//6.10
flatpak-builder --force-clean --user --install-deps-from=flathub \
  flatpak-build io.github.lostjared.AcidCam.yml
```

Test the application directly inside the build sandbox:

```sh
flatpak-builder --run flatpak-build io.github.lostjared.AcidCam.yml acidcam-qt
```

To create an installable bundle, export the build to a local repository first:

```sh
flatpak-builder --force-clean --user --repo=flatpak-repo \
  flatpak-build io.github.lostjared.AcidCam.yml
flatpak build-bundle flatpak-repo AcidCam.flatpak \
  io.github.lostjared.AcidCam
flatpak install --user ./AcidCam.flatpak
flatpak run io.github.lostjared.AcidCam
```

The sandbox grants access to cameras and hardware encoders, networking for
streaming, and the user's Videos and Pictures directories. Update the pinned
`libacidcam` commit in the manifest whenever the application requires a newer
library revision.
