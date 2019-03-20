# nikal
The VISA driver from National Instruments (NI-VISA) for linux systems does not
support linux kernels versioned as 4.x.x.  The problem comes from the kernel
module 'nikal'.  Since NI isn't updating the linux installer, I patched the
sources of nikal kernel module to work with 4.15.x.
