## mload-mod

A `mload-module` mod based on @Leseratte10's [d2xl-cios](https://github.com/Leseratte10/d2xl-cios).

### Features:

**1) Load IOS modules from the SD at MLOAD startup**

Similar to PSP's plugin system, you can create a text file `sd:/mload/mload.txt`. Each line represents the path of an IOS module to load (`.app`). Lines starting with `#` are considered comments. For example:
```
# This is a comment
mload/MYMODULE.app
```

**2) Adds `MLOAD_GET_LOG_BUFFER_AND_EMPTY` `ioctl`**

This `ioctl` is useful for extracting IOS-side logs from the PPC (regular Wii apps) for those who do not have a USB Gecko.

Check out [this repository](https://bitbucket.org/xerpi/mload_test) for a sample homebrew application that uses this `ioctl`.
