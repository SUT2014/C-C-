# REF

REF is a series of applications developed as part of a bigger framework to execute configuration commands on an heavily firewalled test system. 

# Requirements

  - Each Test server can host multiple applications.  Each application is executed as its own linux user.
  - REF should be able to provide a single point of contact but run multiple configuration commands on all the applications running on the back end servers.
  - REF should be able to get back responses for these commands. 
  - Flexibility on the commands run on backend servers, for example, they should be independent of the framework, develop and run unimpeded.
  
# Features!  
  - Listener process acts as a single point of entry to each server.  Multiple Executor process run under different linux users and run the actual commands.
  - Results are routed back to Listener and to clients.
  - GenUI is a generic UI generator using XML files as input.  
  - These XML files define how the UI looks, generally around a "Command" drop down box and a Submit button.  
  - The commands available as part of the drop down box is provided in the XML file.  The results are dumped back onto a text area on the UI screen.

REF has been developed using the following tools/apps:

* [C] - C programming language 
* [MinGW] MinGW libraries have been used, esp Win32 GUI libs.

### Installation

Compile listener and executor using gcc command, preferably on the target server, make file has been provided for GenUI

### Run
A sample configuration file has been provided for the listener and executor application.  
GenUI can take in an XML file, example.xml has been provided.  GenUI is capable of running a remote execution script(ref) on back end servers, example.ref provided.

**Free Software, Hell Yeah!**