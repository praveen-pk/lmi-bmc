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
If the passed commit-id points to a tag, then an archive with the name **openlmi-bmc-\<tag\>.tar.gz** wil be created. This archive can be used with the spec file created in pkg directory to build an rpm package. 


## How to run lmi-bmc 
The post scripts populated in the openlmi-bmc package automatically register the LMI_BMC provider to the CIMOM running. Both sfcb and tog-pegasus CIMOMs are supported. 

With SFCB CIMOM installed on the target system, the following output while enumerating the registered class names confirms the LMI_BMC provider is properly registered
```
 wbemcli ecn https://root:password@localhost:5989/root/cimv2 | grep LMI
 localhost:5989/root/cimv2:LMI_BMC
 ```
wbemcli is a standalone WBEM client program. To enumerate the Service Processors installed in the system:

``` 
wbemcli ei https://root:password@localhost:5989/root/cimv2:LMI_BMC
```
 
