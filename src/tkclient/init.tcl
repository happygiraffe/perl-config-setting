######################################################################
# cmds.tcl
#
# Initialization script.  Reads in user profiles, and whatever.
#
# Copyright 1996 Dominic Mitchell
# 

#
# $Header: /home/ncvs/src/tkclient/init.tcl,v 1.2 1997/02/13 17:08:45 dom Exp $
#

###
### Here are the defaults:
###

set LoginName ""
set Password  ""
set SpiderHost "localhost"
set SpiderPort "4321"
set AcceptPages 1

set SpiderChan 	{}

###
### Here, we load in a .tkclientrc file, if it exists.
###

if {$tcl_platform(platform) == "unix"} {
    set ConfigFile "~/.tkclientrc"
} else {
    set ConfigFile "tkclient.rc"
}
if [file readable $ConfigFile] {
    source $ConfigFile
}


###
### Set up various handlers to get reasonable default behaviour.
###

set InitHandlers(LoginCheck) {
    if {$LoginName == "" || $Password == ""} {
        DlgHostDetails
    }
}

#
# Run each of these procedures whenever we are newly connected
#
set ConnectHandlers(MOTD) {
    DisplayMOTD
}
set ConnectHandlers(Page) {
    global AcceptPages AsyncHandlers pagetag
    if $AcceptPages {
        set reply [DoCmd [list "PAGE ON"]]
        set pagetag [lindex $reply 1]
        set AsyncHandlers($pagetag) DlgPage
    }
}
set ConnectHandlers(Areas) {
    .bt.mesg invoke
    MsgInitLastRead
}
set ConnectHandlers(ChangeFileMenu) {
    # Switch the connect button to a disconnect one
    .m.file.m entryconfigure 0 \
            -label Disconnect \
            -command MFileDisConnect
}
set ConnectHandlers(Listener) {
    global SpiderChan
    fileevent $SpiderChan readable Listener
}

#
# Run each of these whenever we are disconnecting from Spider.
#
set DisConnectHandlers(ChangeFileMenu) {
    # Switch the disconnect button to a connect one
    .m.file.m entryconfigure 0 \
            -label Connect \
            -command MFileConnect
}
set DisConnectHandlers(Label) {
    .af.area_label configure -text "Not Connected"
}
set DisConnectHandlers(Listener) {
    global SpiderChan
    # Don't even think about having fileevents when we're offline
    fileevent $SpiderChan readable {}
}
set DisConnectHandlers(Page) {
    global AcceptPages AsyncHandlers pagetag
    if $AcceptPages {
        set reply [DoCmd [list "PAGE OFF"]]
        unset AsyncHandlers($pagetag)
    }
}
