# lmi-bmc


lmi-bmc is a CIM-XML provider based on openlmi framework. This provider publishes information about the service processor via CIM-XML. 
The information currently includes:
* IP addressess
* MAC address
* VLAN id
* Firmware Version
* List of Supported Protocols
* Interface mode (Shared vs dedicated)
  
## How to build and run from lmi-bmc provider
The pkg/openlmi-bmc.spec.skel file captures all the build requirements for this provider. Be sure to install all those requirements on your build host before staring the build.
After installing all the build requirements follow the commands below:
```
git clone https://github.com/praveen-pk/lmi-bmc.git 
cd lmi-bmc
mkdir build; cd build

cmake ../
make 
make install
```
## How to package lmi-bmc
```
cd lmi-bmc
./make-release.sh <commit-id>
```
TODO: Needs to be corrected
Pass the latest commit ID to the script. The script will pick up the latest tagged commit in the history starting from the commit ID provided and create a dist archive.
Use the thus created dist archive with the spec file in pkg directory and create an rpm package.

