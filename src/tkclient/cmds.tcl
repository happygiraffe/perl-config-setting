######################################################################
# cmds.tcl
#
# Provides commands to action all the widgets in the main user interface. 
#
# Copyright 1996 Dominic Mitchell
# 

#
# $Header: /home/ncvs/src/tkclient/cmds.tcl,v 1.2 1997/02/13 17:08:44 dom Exp $
#

######################################################################
# MFileConnect
#
# Menu command for File/Connect.  Establish a connection to the Spider
# server type thingy .
#
proc MFileConnect {} {
    global SpiderHost SpiderPort SpiderChan LoginName Password
    global ConnectHandlers

    set SpiderChan [socket $SpiderHost $SpiderPort]
    fconfigure $SpiderChan -buffering line
    fconfigure $SpiderChan -translation crlf

    # Ensure that we're talking to a Spider...
    set line [gets $SpiderChan]
    if {[lindex $line 0] != "Spider"} {
        tk_dialog .foo "Bad Port#" "The port number that you gave is \
                invalid.  Please enter another one using the \
                File/Options dialog." \
                warning 0 OK
        close $SpiderChan
        set SpiderChan {}
        return
    }

    # Now, log us in
    PutMesg [list "login $LoginName $Password"]
    set reply [GetMesg]
    if {[ReplyCode $reply] != 220} {
        tk_dialog .foo "Problem" "Spider said:\n$reply" \
                warning 0 OK
        close $SpiderChan
        set SpiderChan {}
        return
    }

    # Let user know - should really be a bitmap on the main display panel
    tk_dialog .foo "You're in" "Successfully connected" info 0 OK

    # Run each Connection handler, in any order
    foreach i [array names ConnectHandlers] {
        eval $ConnectHandlers($i)
    }
}

######################################################################
# MFileDisConnect
#
# Disconnect the user from the BBS
#
proc MFileDisConnect {} {
    global SpiderChan DisConnectHandlers

    # Run each DisConnection handler, in any order
    foreach i [array names DisConnectHandlers] {
        eval $DisConnectHandlers($i)
    }

    DoCmd [list "QUIT"]
    set SpiderChan {}
}

######################################################################
# MFileExit
#
# Menu command for File/Exit.  Log off the BBS and exit the user
# interface. 
#
proc MFileExit {} {
    if [Connected] {
        MFileDisConnect
    }
    destroy .
}

######################################################################
# MOptionAcceptPages
#
# Change the state of paging to this user.
#
proc MOptionAcceptPages {} {
    global AcceptPages AsyncHandlers pagetag
    
    if $AcceptPages {
        if {! [Connected]} {
            return
        }
        set reply [DoCmd [list "PAGE ON"]]
        set pagetag [lindex $reply 1]
        set AsyncHandlers($pagetag) DlgPage
    } else {
        if {! [Connected]} {
            return
        }
        DoCmd [list "PAGE OFF"]
        unset AsyncHandlers($pagetag)
    }
}

######################################################################
# MOptionSave
#
# Save all the Options to ConfigFile
#
proc MOptionSave {} {
    global ConfigFile
    global LoginName Password SpiderHost SpiderPort AcceptPages
    global MsgLastRead

    set f [open $ConfigFile "w"]
    puts $f "#"
    puts $f "# Options file for TkClient.  Automatically generated."
    puts $f "# Do not edit, changes will be lost"
    puts $f "#"
    puts $f ""
    puts $f "set LoginName   \"$LoginName\""
    puts $f "set Password    \"$Password\""
    puts $f "set SpiderHost  \"$SpiderHost\""
    puts $f "set SpiderPort  \"$SpiderPort\""
    puts $f "set AcceptPages \"$AcceptPages\""
    if [info exists MsgLastRead] {
        puts $f ""
        foreach i [array names MsgLastRead] {
            puts $f "set MsgLastRead($i)\t\"$MsgLastRead($i)\""
        }
    }
    close $f
}

######################################################################
# MHelpAbout
#
# Menu command for Help/About.  Usual crap. 
#
proc MHelpAbout {} {
    tk_dialog .foo "About tkclient 1.0" \
            "Version 1.0\n
Written by \n
Dominic Mitchell (dom@myrddin.demon.co.uk)\n
and\n
Matt Davidson (mndavidson@plym.ac.uk" \
            info 0 OK
}

######################################################################
# BMessages
#
# Code for the messages button.
#
proc BMessages {} {
    if {! [Connected]} {
        return
    }

    global AfAreaState
    if {"$AfAreaState" != "areas"} {
        # Find out what discussion areas we actually have.
        # XXX - getars shouldn't have to be lowercase
        set areas [DoCmd [list "getars"]]
        if [TestForError $areas] {
            return
        }
        set areas [lrange $areas 1 end]
        
        # And update the display
        .af.area_label configure -text "Discussion Areas"
        .af.area_label configure -width 30
        .af.area_list delete 0 end
        .af.area_list configure -font variable
        foreach i $areas {
            .af.area_list insert end $i
        }
        .af.area_list selection set 0
        .af.area_list activate 0
        set AfAreaState areas
        
        # Recursive callback!
        bind .af.area_list <Double-Button-1> {
            .bt.mesg invoke
        }
    } else {
        # Check to see if we already have one open
        if [winfo exists .msg] {
            return
        }
        # Check to see if we actually have the selection where its needed
        set tmp [.af.area_list get active]
        if {$tmp == {} } {
            set tmp [.af.area_list get [.af.area_list curselection]]
            if {$tmp == {}} {
                return
            }
        }
        # Start up the reader, based on the selection
        DlgMessages $tmp
    }

}

######################################################################
# BTalk
#
# Code for the talker button.
#
proc BTalk {} {
    if {! [Connected]} {
        return
    }

    if [winfo exists .talk] {
        return
    }
    DlgTalker 0
}

######################################################################
# BWho
#
# Code for the who button.
#
proc BWho {} {
    if {! [Connected]} {
        return
    }

    # Retrieve logged on user list
    set wholist [DoCmd [list "WHO"]]
    if [TestForError $wholist] {
        return
    }
    # Get rid of the error code
    set headers [lindex $wholist 1]
    set wholist [lrange $wholist 2 end]

    #XXX - replace with a frame with grid management
    .af.area_label configure -text "Current Users"
    .af.area_list delete 0 end
    # .af.area_list configure -font "*-*-medium-r-*--*-120-*-*-m-*-*-*"
    .af.area_list configure -font fixed
    set fmt "%-20s %-10s %-9s %-20s"
    set headers [eval format {$fmt} $headers]
    .af.area_list insert end $headers
    .af.area_list configure -width [string length "$headers"]
    foreach i $wholist {
        .af.area_list insert end [eval format {$fmt} $i]
    }
    # bind a doubleclick to the "Page" function
    bind .af.area_list <Double-Button-1> {
        set tmp [.af.area_list get active]
        if {$tmp == {} } {
            set tmp [.af.area_list get [.af.area_list curselection]]
            if {$tmp == {}} {
                return
            }
        }
        DlgPageTo [lindex $tmp 0]
    }

    global AfAreaState
    set AfAreaState who
}

######################################################################
# BPage
#
# Code for the pager button.
#
proc BPage {} {
    if {! [Connected]} {
        return
    }
    DlgPageTo {}
}

proc ToBeDone name {
    tk_dialog .foo \
            "Ooops" \
            "Sorry, I haven't written $name yet!" \
            warning \
            0 \
            OK
}

######################################################################
# BFortune
#
# Display a silly quote.
#
proc BFortune {} {
    if {! [Connected]} {
        return
    }

    set fortune [DoCmd [list "QOTD"]]
    if [TestForError $fortune] {
        return
    }
    set fortune [lrange $fortune 1 end]

    tk_dialog .fortune "Fortune Cookie" [join $fortune "\n"] \
            {} 0 OK
}
