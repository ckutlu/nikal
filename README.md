# nikal
The VISA driver from National Instruments (NI-VISA) for linux systems does not
support linux kernels versioned as 4.x.x.  The problem comes from the kernel
module 'nikal'.  Since NI isn't updating the linux installer, I patched the
sources of nikal kernel module to work with 4.20.x.


# How to install?
1. Download the latest NI-VISA drivers *that are supported for CentOS/Red Hat
   systems*.  The file name is NI-VISA-17.0.0.iso.

2. Become root by:
```
$ sudo su
[enter your sudo password here]
```

3. Go to root's home:
```
$ cd ~
```

4. Make a temporary folder:
```
$ mkdir tmp
```

5. Mount the iso file:
```
$ mount /path/to/downloaded/file.iso tmp
```

6. Make a folder to hold the contents of the iso, and copy stuff in tmp.
```
$ mkdir ~/nivisa && cp -r tmp nivisa
```

7. Unmount tmp:
```
$ umount ~/tmp
```

8. Go to nivisa folder and run ./INSTALL:
```
$ cd nivisa
$ ./INSTALL
```

9. Checkout FAQ.1 if you encounter a problem here.

10. You will be prompted to reboot after installation, do it.

11. You need to run 'updateNIDrivers' after installation.  This is where the
    fun begins.  Make sure you become root again.

12. Install kernel headers, make tool, 
```
$ dnf install kernel-devel make
```

13. Download the patched nikal files and install it at the appropriate
    locations.  
```
$ cd ~
$ wget https://github.com/ckutlu/nikal/archive/master.zip
$ unzip master.zip
$ cp -r nikal-master/nikal /usr/local/natinst/nikal/src/
```

14. If you tried running updateNIDrivers before, there may be remaining cached
    files would be used instead of the patched sources we just installed.
    Remove them via(BE CAREFUL WITH THE rm -rf COMMAND, SUBSTITUTE WITH mv IF YOU
    FEEL UNCOMFORTABLE): 
```
$ rm -rf /var/lib/nikal/$(uname -r)
```

15. Now "update" the NI drivers (this builds up NI driver kernel modules and
    installs them).  You have to do this whenever you update your kernel in any
    way.
```
$ updateNIDrivers
```

16. Reboot after finishing.

17. Remember to call updateNIDrivers whenever you do a kernel upgrade.


# Why is it like this?
1. The NI-VISA drivers are mainly proprietary/closed source.  Therefore, you are
   tied to the vendor's policies which are generally business driven.  Since
   linux userbase is small, they usually get away with not fixing bugs in their
   drivers, and not providing good customer support.  So, expect errors, and be
   prepared to solve them.

2. The drivers come with kernel modules that are to be built using the kernel
   headers that belongs to the running system.  Thus, whenever you update your
   kernel, you need to update the drivers.  This can be automatized in principle
   (with a very simple script put in the correct folder), however due to the 1.
   above the module sources get broken once in a while.

3. With newer kernels the installer may break.  The breakage is due to the
   updated kernels deprecating some functionality or adding something that
   breaks the NI's configure script.  Usually, the bugs are simple enough to fix
   for a person who only has C and bash familiarity, with no considerable
   understanding of the linux kernel nor the NI's kernel modules.


# FAQ
1. I run the ./INSTALL script and I get 'package ni*blabla* does not verify: no digest" errors and installer gets aborted.  What do I do?  Why: This happens because rpm by default tries to verify the rpm by comparing with a known digest file.  The NI-VISA, being sloppy as they are, do not provide these digest files.
Solution: Disable verification in rpm during installation process. Open INSTALL
file with your favorite text editor.  Find the line that has:

> RPMOpts="$RPMOpts -Uvh"

Change it into:

> RPMOpts="$RPMOpts - Uvh --noverify"
