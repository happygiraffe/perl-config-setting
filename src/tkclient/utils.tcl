######################################################################
# utils.tcl
#
# Provides miscellaneous utility functions
#
# Copyright 1996 Dominic Mitchell
# 

#
# $Header: /home/ncvs/src/tkclient/utils.tcl,v 1.2 1997/02/13 17:08:50 dom Exp $
#

######################################################################
# DoCmd
#
# Run a command through Spider, taking care of any unsolicited messages
# that might pass by.
#
proc DoCmd {msg} {
    global SpiderChan

    if {! [Connected]} {
        return
    }
    
    # Stop fileevents from occuring
    set oldevent [fileevent $SpiderChan readable]
    fileevent $SpiderChan readable {}

    PutMesg $msg
    set reply [GetMesg]
    while {[ReplyCode $reply] == 280} {
        # Handle any unsolicited messages that might occur...
        AsyncMesgMgr $reply
        set reply [GetMesg]
    }
    # Re-enable fileevents
    fileevent $SpiderChan readable $oldevent
    return $reply
}

######################################################################
# GetMesg
#
# Get a message from Spider.  Remove trailing "." and perform ".."->"."
# mapping as well.
#
proc GetMesg {} {
    global SpiderChan
    set reply {}

    set line [gets $SpiderChan]
    while {$line != "."} {
        if {$line == ".."} {
            set line "."
        }
        lappend reply $line
        set line [gets $SpiderChan]
    }
    return $reply
}


######################################################################
# PutMesg
#
# Send out a message to Spider.  Includes "."->".." mapping and final
# "." adding.
#
proc PutMesg {msg} {
    global SpiderChan

    foreach line $msg {
        if {$line == "."} {
            set line ".."
        }
        puts $SpiderChan $line
    }
    puts $SpiderChan "."
}

######################################################################
# TestForError
#
# Returns true/false if the reply is in error.  Displays the error if
# it exists.
#
proc TestForError {msg} {
    if {[ReplyCode $msg] != 211} {
        set line [lindex $msg 0]
        tk_dialog .foo "Problem" "Spider said:\n$line" \
                warning 0 OK
        return 1
    } else {
        return 0
    }
}

######################################################################
# ReplyCode
#
# Returns the first word of msg (usually a reply code).
#
proc ReplyCode {msg} {
    return [lindex [lindex $msg 0] 0 ]
}

######################################################################
# Connected
#
# Return true (1) if we are on-line, false (0) elsewise.
#
proc Connected {} {
    global SpiderChan

    if {$SpiderChan == {}} {
        return 0
    } else {
        return 1
    }
}

######################################################################
# Listener
#
# Listen out for unsolicited messages and pass them to AsyncMesgMgr,
# below to be processed.  For some reason, we get called when we
# disconnect, as well.
#
proc Listener {} {
    if [Connected] {
        AsyncMesgMgr [GetMesg]
    }
}

######################################################################
# AsyncMesgMgr
#
# Gets called either through a fileevent or DoCmd and processes the
# message passed as a parameter.  If it is unknown then drop it and
# return.
#
proc AsyncMesgMgr {mesg} {
    global AsyncHandlers

    set tag [lindex [lindex $mesg 0] 1]
    set mesg [lrange $mesg 1 end]

    if {$AsyncHandlers($tag) != {}} {
        eval $AsyncHandlers($tag) [list $mesg]
    }
}

######################################################################
# DisplayMOTD
#
# Retrieve & display the Message-of-the-day
#
proc DisplayMOTD {} {
    if {! [Connected]} {
        return
    }
    
    set result [DoCmd [list "MOTD"]]

    if [TestForError $result] {
        return
    }

    set motd [lrange $result 1 end]
    set motd [join $motd "\n"]

    # Display the damn thing... XXX - 7" is a kludge
    option add *Dialog.msg.wrapLength 7i
    tk_dialog .motd "Message Of The Day" $motd {} 0 OK
    # Put it back to it's default value
    option add *Dialog.msg.wrapLength 3i
}
