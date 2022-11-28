Latest Update Directory

OTAUpdater to ```download_and_install_update_if_available()``` the pending version, it will:

see which version it must download from the ```.version_on_reboot``` file in the ```next``` folder
download all the ```new .py files in the next folder```
```delete the main folder```
rename the next folder to main
rename the .version_on_reboot to .version
and do a machine.reset().

In your main.py file you then continue as normal, importing and bootstrapping your code from your main module.
The .version file is used to compare it against the latest release on your GitHub account.
