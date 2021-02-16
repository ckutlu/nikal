# About
The 2020 VISA driver from National Instruments (NI-VISA) for linux systems does
not support linux kernels versioned as 5.10.x.  The problem comes from the
kernel module 'nikal'.  This repo contains patches required to make it work for
for a Fedora 33 system running linux kernel 5.10.15.

# Disclaimer
The author of this patch has no deep knowledge of the linux kernel.  The only
thing he did was to navigate the 4.x and 5.x linux source trees for the related
errors when trying to build nikal module from ni source files.  He then applied
changes according to the changes in the linux kernel for the relevant
functions.  Since the changes appeared to be superficial like renaming of
entities, or using slightly different structures for data, the task was not
very difficult.  Having said that, these patches come with NO warranty
whatsoever. The website [1][https://elixir.bootlin.com] was used to browse
linux kernel sources.


# Install
To apply the changes, download this repo in some folder named `tmp` and copy
the `nikal.c` to destination.  I.e.:

```bash
NIVERSION="20.0.0f0"
git clone https://github.com/ckutlu/nikal.git
rm -r /usr/src/nikal-$NIVERSION
cp -r tmp/kernel-5.10.15/nikal-$NIVERSION /usr/src/
```

You can now reinstall the kernel modules necessary for the lib operation.

```bash
dkms autoinstall
```

# More
Check out old branches for different ni & kernel combinations.

