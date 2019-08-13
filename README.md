# Installation of NI-VISA
Follow the official instructions for installing the libraries.

# Failing Build
Installation of some modules may fail depending on the kernel version.
Usually the culprit is `nikal` module.  The source for this kernel module is
usually located under `/usr/src/nikal-19.2.0f0`.

To apply the changes, download this repo somewhere and copy the `nikal.c` to
destination.  Note that taking backup of the original file is your
responsibility.

```bash
pushd /tmp
NIVERSION="19.2.0f0"
git clone https://github.com/ckutlu/nikal.git
cp nikal/nikal/nikal.c /usr/src/nikal-$NIVERSION
```
You can now reinstall the kernel modules necessary for the lib operation.

```bash
dkms autoinstall
```
