######################################################################
# talker.tcl
#
# Contains all code to do with the talker.
#
# Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
#

#
# $Header: /home/ncvs/src/tkclient/talker.tcl,v 1.2 1997/02/13 17:08:47 dom Exp $
#

######################################################################
# DlgTalker
#
# Sets up the Talker dialog box.  Parameter is the initial channel to
# join.
#
proc DlgTalker {initchan} {
    if {! [Connected]} {
        return
    }

    ## Set up the frame, first of all.
    toplevel .talk

    # Channel Number
    frame .talk.chan
    label .talk.chan.l \
            -text "Channel:"
    entry .talk.chan.ent \
            -width 3 \
            -font 9x15
    button .talk.chan.who \
            -text Who \
            -command TlkWho
    button .talk.chan.close \
            -text Close \
            -command TlkClose

    pack .talk.chan \
            -padx 2 \
            -pady 2 \
            -fill both
    pack .talk.chan.close \
            -side right
    pack .talk.chan.who \
            -side right
    pack .talk.chan.l \
            -side left
    pack .talk.chan.ent \
            -side left \
            -anchor w

    # Main talker window
    frame .talk.main
    scrollbar .talk.main.scroll \
            -command ".talk.main.text yview"
    text .talk.main.text \
            -yscrollcommand ".talk.main.scroll set" \
            -state disabled \
            -width 80 \
            -height 20
    pack .talk.main \
            -expand 1 \
            -fill both \
            -padx 10 \
            -pady 10
    pack .talk.main.scroll \
            -side right \
            -fill y
    pack .talk.main.text \
            -expand 1 \
            -fill both

    # Your input.
    frame .talk.reply
    label .talk.reply.l \
            -text "Say:" 
    entry .talk.reply.ent \
            -width 70
    pack .talk.reply \
            -fill x
    pack .talk.reply \
            -padx 2 \
            -pady 2
    pack .talk.reply.l \
            -side left
    pack .talk.reply.ent \
            -side left \
            -expand 1 \
            -fill x

    # Install the default channel to whatever we need
    if {$initchan == {}} {
        set initchan 0
    }
    global oldchan
    set oldchan -1
    .talk.chan.ent delete 0 end
    .talk.chan.ent insert end $initchan
    TlkChangeChan

    ## Now, set up event handlers
    bind .talk.chan.ent <Key-Return> TlkChangeChan
    bind .talk.reply.ent <Key-Return> TlkSpeak
    global AsyncHandlers DisConnectHandlers ConnectHandlers
    # XXX - tmesg is hardcoded by talkermod...
    set AsyncHandlers(tmesg) TlkHear
    set DisConnectHandlers(tmesg) TlkStop
    set ConnectHandlers(tmesg) TlkStart
}

######################################################################
# TlkClose
#
# Quit the talker.
#
proc TlkClose {} {
    if [Connected] {
        set chan [.talk.chan.ent get]
        DoCmd [list "synoff $chan"]
    }
    global AsyncHandlers DisConnectHandlers ConnectHandlers
    unset AsyncHandlers(tmesg)
    if {[lsearch [array names ConnectHandlers] tmesg] != -1} {
        unset ConnectHandlers(tmesg)
    }
    if {[lsearch [array names DisConnectHandlers] tmesg] != -1} {
        unset DisConnectHandlers(tmesg)
    }
    destroy .talk
}

######################################################################
# TlkWho
#
# Put up a list of who's on the current channel.
#
proc TlkWho {} {
    if {! [Connected]} {
        return
    }

    set chan [.talk.chan.ent get]
    set talkers [DoCmd [list "clist $chan"]]

    if [TestForError $talkers] {
        return
    }
    set talkers [lrange $talkers 1 end]
    set talkers [join $talkers " "]

    .talk.main.text configure -state normal
    .talk.main.text insert end "On this channel: $talkers\n"
    .talk.main.text configure -state disabled
}

######################################################################
# TlkChangeChan
#
# Change the channel that we are currently logged onto.
#
proc TlkChangeChan {} {
    if {! [Connected]} {
        return
    }

    global oldchan
    set number [.talk.chan.ent get]
    if {($number >= 0 ) && ($number <= 256)} {
        if {$oldchan != -1} {
            set reply [DoCmd [list "synoff $oldchan"]]
            if [TestForError $reply] {
                .talk.chan.ent delete 0 end
                .talk.chan.ent insert end $oldchan
                return
            }
        }
        set reply [DoCmd [list "join $number"]]
        if [TestForError $reply] {
            .talk.chan.ent delete 0 end
            .talk.chan.ent insert end $oldchan
            return
        }
        set oldchan $number
    } else {
        .talk.chan.ent delete 0 end
        .talk.chan.ent insert end $oldchan
        return
    }
}

######################################################################
# TlkSpeak
#
# Send a message over the channel.
#
proc TlkSpeak {} {
    if {! [Connected]} {
        return
    }

    set wisdom [.talk.reply.ent get]
    set number [.talk.chan.ent get]
    .talk.reply.ent delete 0 end
    if {"$wisdom" == ""} {
        return
    }
    DoCmd [list "bchan $number $wisdom"]
    # The AsyncHandler will take care of displaying it for us...
}

######################################################################
# TlkHear
#
# The AsyncHandler for the talker.
#
proc TlkHear {msg} {
    set number [.talk.chan.ent get]
    if {$number != [lindex $msg 0]} {
        return
    }

    set name [lindex $msg 1]
    set text [lindex $msg 2]

    .talk.main.text configure -state normal
    .talk.main.text insert end "$name: $text\n"
    .talk.main.text configure -state disabled
}

######################################################################
# TlkStop
#
# Handles signing us off a channel when we disconnect.
#
proc TlkStop {} {
    if {! [Connected]} {
        return
    }

    set number [.talk.chan.ent get]
    DoCmd [list "synoff $number"]
    bind .talk.reply.ent <Key-Return> {}
}

######################################################################
# TlkStart
#
# Restarts the talker after a disconnection.
#
proc TlkStart {} {
    if {! [Connected]} {
        return
    }

    set number [.talk.chan.ent get]
    DoCmd [list "join $number"]
    bind .talk.reply.ent <Key-Return> TlkSpeak
}
