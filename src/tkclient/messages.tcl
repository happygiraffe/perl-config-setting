######################################################################
# messages.tcl
#
# Provides commands to deal with the messaging facility in Spider.
# (module msgmod)
#
# Copyright 1996 Dominic Mitchell
# 

#
# $Header: /home/ncvs/src/tkclient/messages.tcl,v 1.1 1997/02/13 17:04:56 dom Exp $
#
# $Log: messages.tcl,v $
# Revision 1.1  1997/02/13 17:04:56  dom
# Initial revision
#
# Revision 1.0  1996/05/14 08:44:12  dom
# Initial revision
#
#

######################################################################
# DlgMessages
#
# Sets up a messaging window.  Parameter is the area in which we are
# interested in reading about.
#
proc DlgMessages {area} {

    if {! [Connected]} {
        return
    }

    # Everything is relative to this.
    set base .msg

    # Create the UI.
    toplevel $base
    wm title $base "Reading $area"
    wm group .msg .
    frame $base.bb
    frame $base.h
    frame $base.msg

    # Button bar
    button $base.bb.next \
            -command "MsgNext $base" \
            -text "Next"
    button $base.bb.prev \
            -command "MsgPrev $base" \
            -text "Prev"
    button $base.bb.reply \
            -command "MsgReply $base" \
            -text "Reply"
    button $base.bb.new \
            -command "MsgNew $base" \
            -text "New"
    button $base.bb.unread \
            -command "MsgUnRead $base" \
            -text "UnRead"
    button $base.bb.update \
            -command "MsgUpdate $base" \
            -text "Update"
    button $base.bb.close \
            -command "MsgClose $base" \
            -text "Close"

    pack $base.bb -fill x
    pack $base.bb.next $base.bb.prev $base.bb.reply \
            $base.bb.new -side left -padx 2
    pack $base.bb.close $base.bb.update $base.bb.unread -side right

    # Header window
    listbox $base.h.list \
            -yscrollcommand "$base.h.scroll set" \
            -height 5 \
            -width 80
    scrollbar $base.h.scroll \
            -command "$base.h.list yview"
    # This is never packed - It's hidden from view as its only needed by us
    # It's sort of like a global variable for mirroring $base.h.list
    listbox $base.h.ids
    # Another hidden widget...
    entry $base.h.area
    $base.h.area insert end "$area"

    pack $base.h -fill x
    pack $base.h.scroll -side right -fill y
    pack $base.h.list -fill x -expand 1

    # Message-text window
    text $base.msg.text \
            -yscrollcommand "$base.msg.scroll set" \
            -height 20
    scrollbar $base.msg.scroll \
            -command "$base.msg.text yview"

    pack $base.msg -expand 1 -fill both
    pack $base.msg.scroll -side right -fill y
    pack $base.msg.text -expand 1 -fill both

    # Set up the necessary bindings
    bind $base.h.list <Button-1> "+MsgView $base"

    # Fill in the details, now we've somewhere to put them.
    $base.bb.update invoke
}

######################################################################
# MsgUpdate
#
# Fill in the various bits with the results of a cmd.
#
proc MsgUpdate {root} {
    global MsgLastRead

    set area [$root.h.area get]
    set newid [$root.h.ids get end]
    if {$newid == {} } {
        set newid $MsgLastRead($area)
    }
    incr newid
    set newid [format "%08d" $newid]

    set reply [DoCmd [list "gethed post $area $newid"]]
    if [TestForError $reply] {
        puts "But msgid was $newid"
        return
    }
    set reply [lrange $reply 1 end]
    if {$reply == {} } {
        tk_dialog .nonew "Nothing here" \
                "There are no new messages in this area" \
                info 0 OK
        return
    }

    # And fill in the new messages
    if [info exists MsgArr] {
        unset MsgArr
    }
    foreach {i j k l m} $reply {
        foreach x [list $i $j $k $l] {
            if {[lindex $x 0] == "From:" } {
                set from [lindex $x 1]
            }
            if {[lindex $x 0] == "Subject:" } {
                set subject "[lrange $x 1 end]"
            }
            if {[lindex $x 0] == "Id:" } {
                set id [lindex $x 1]
            }
        }
        set val [format "%-15s %s" $from $subject]
        set MsgArr($id) $val
    }
    foreach i [lsort -integer [array names MsgArr]] {
        $root.h.list insert end $MsgArr($i)
        $root.h.ids insert end $i
    }
    $root.h.list selection set 0
    $root.h.list activate 0

    # Display the first entry
    MsgView $root
}

######################################################################
# MsgUnRead
#
# Unread the last 50 Messages.
#
proc MsgUnRead {root} {
    global MsgLastRead

    set area [$root.h.area get]
    set oldid [$root.h.ids get 0]
    if {$oldid == {} } {
        # Guaranteed to be 0 if nothing else
        set oldid $MsgLastRead($area)
    }
    if {$oldid == 0} {
        return
    }
    if {$oldid < 50} {
        set oldid 0
    } else {
        incr oldid -50
    }
    set oldid [format "%08d" $oldid]

    set reply [DoCmd [list "gethed post $area $oldid"]]
    if [TestForError $reply] {
        return
    }
    set reply [lrange $reply 1 end]
    if {$reply == {} } {
        tk_dialog .nonew "Nothing here" \
                "There are no messages in this area" \
                info 0 OK
        return
    }

    # And fill in the new messages
    $root.h.list delete 0 end
    $root.h.list delete 0 end
    $root.msg.text delete 0.0 end
    if [info exists MsgArr] {
        unset MsgArr
    }
    foreach {i j k l m} $reply {
        foreach x [list $i $j $k $l] {
            if {[lindex $x 0] == "From:" } {
                set from [lindex $x 1]
            }
            if {[lindex $x 0] == "Subject:" } {
                set subject "[lrange $x 1 end]"
            }
            if {[lindex $x 0] == "Id:" } {
                set id [lindex $x 1]
            }
        }
        set val [format "%-15s %s" $from $subject]
        set MsgArr($id) $val
    }
    foreach i [lsort -integer [array names MsgArr]] {
        $root.h.list insert end $MsgArr($i)
        $root.h.ids insert end $i
    }
    $root.h.list selection set 0
    $root.h.list activate 0

    # Rewind the counter of seen messages
    set MsgLastRead($area) $oldid

    # Display the first entry
    MsgView $root
}

######################################################################
# MsgInitLastRead
#
# Initialize the "last read" number for each of the areas
#
proc MsgInitLastRead {} {
    global MsgLastRead

    if {! [Connected]} {
        return
    }

    set areas [DoCmd [list "getars"]]
    if [TestForError $areas] {
        return
    }
    set areas [lrange $areas 1 end]

    # Check that the damn thing is there, to start with.
    if {[array exists MsgLastRead] == 0} {
        foreach i $areas {
            set MsgLastRead($i) [format "%08d" 0]
        }
        return
    }
    # Now, make sure that each area is in the array
    foreach i $areas {
        if {[lsearch [array names MsgLastRead] $i] == -1} {
            set MsgLastRead($i) [format "%08d" 0]
        }
    }
}

######################################################################
# MsgClose
#
# Shut down this message window
#
proc MsgClose {root} {
    # Take note of what we have seen...
    global MsgLastRead ConfigFile
    set area [$root.h.area get]
    set lastid $MsgLastRead($area)

    set f [open $ConfigFile "a"]
    puts $f "set MsgLastRead($area) \"$lastid\""
    close $f

    destroy $root
}

######################################################################
# MsgNext
#
# Moves onto the next message if there is one
#
proc MsgNext {root} {
    set cur [$root.h.list index active]
    if {$cur == {} } {
        set cur [$root.h.list get [$root.h.list curselection]]
        if {$cur == {}} {
            return
        }
    }
    incr cur
    if {$cur < [$root.h.list size]} {
        $root.h.list selection clear [expr $cur - 1]
        $root.h.list selection set $cur
        $root.h.list activate $cur
        $root.h.list see $cur
        MsgView $root
    }
}

######################################################################
# MsgPrev
#
# Moves back to the previous message if there is one
#
proc MsgPrev {root} {
    set cur [$root.h.list index active]
    if {$cur == {} } {
        set cur [$root.h.list get [$root.h.list curselection]]
        if {$cur == {}} {
            return
        }
    }
    incr cur -1
    if {$cur >= 0} {
        $root.h.list selection clear [expr $cur + 1]
        $root.h.list selection set $cur
        $root.h.list activate $cur
        $root.h.list see $cur
        MsgView $root
    }
}

######################################################################
# MsgView
#
# Puts the message associated with Msgid into the viewing window.
#
proc MsgView {root} {
    global MsgLastRead

    set area [$root.h.area get]
    set id [$root.h.ids get [$root.h.list index active]]
    if {$id == {} } {
        set id [$root.h.ids get [$root.h.list curselection]]
        if {$cur == {}} {
            return
        }
    }
    set reply [DoCmd [list "getmsg post $area $id"]]
    set reply [lrange $reply 1 end]

    # Install the text of the message
    $root.msg.text configure -state normal
    $root.msg.text delete 0.0 end
    set reply [join $reply "\n"]
    $root.msg.text insert end $reply
    $root.msg.text configure -state disabled

    # Update the last read variable for this area
    set id [$root.h.ids get [$root.h.list index active]]
    set area [$root.h.area get]
    if {$MsgLastRead($area) < $id} {
        set MsgLastRead($area) $id
    }
}

######################################################################
# MsgNew
#
# Add a new message to the message base.
#
proc MsgNew {root} {
    set area [$root.h.area get]
    MsgEditor "$area" {} {} $root.bb.update
}

######################################################################
# MsgReply
#
# Reply to an existing message
#
proc MsgReply {root} {
    set area [$root.h.area get]
    set tmp [$root.h.list get active]
    set author [lindex $tmp 0]
    set subject [lrange $tmp 1 end]
    if {[regexp -nocase -- ^re: "$subject"] == 0} {
        set subject "Re: $subject"
    }

    # Prepare the text for a reply.
    set text "$author wrote:"
    foreach line [split [$root.msg.text get 1.0 end] "\n"] {
        set text [join [list $text "> $line"] "\n"]
    }
    MsgEditor "$area" "$text" "$subject" $root.bb.update
}

######################################################################
# MsgEditor
#
# Edit text in a window and send it off to Spider
#
proc MsgEditor {area text subject updater} {
    set base .newmsg
    toplevel $base
    wm title $base "Editor"
    frame $base.top
    frame $base.bot
    frame $base.subj

    label $base.subj.lbl \
            -text "Subject:"
    entry $base.subj.ent
    text $base.top.text \
            -yscrollcommand "$base.top.scroll set" \
            -height 20
    scrollbar $base.top.scroll \
            -command "$base.top.text yview"
    button $base.bot.post \
            -command "MsgBPost $base $area $updater" \
            -text Post
    button $base.bot.can \
            -command "destroy $base" \
            -text Cancel

    pack $base.subj -fill x -expand 1
    pack $base.subj.lbl -side left
    pack $base.subj.ent -side left -fill x -expand 1
    pack $base.top -expand 1 -fill both
    pack $base.bot -fill x -expand 1
    pack $base.top.scroll -side right -fill y
    pack $base.top.text -expand 1 -fill both
    pack $base.bot.post $base.bot.can -side left -pady 2 -expand 1
    
    # Fill in with what we have
    $base.subj.ent insert end "$subject"
    $base.top.text insert end "$text"
}

######################################################################
# BPost
#
# Posts a message in the MsgEditor window into Spider
#

proc MsgBPost {root area updater} {
    set subject "[$root.subj.ent get]"
    set message "putmsg post $area $subject"
    set text [split [$root.top.text get 0.0 end] "\n"]

    set reply [DoCmd [eval list {$message} $text]]
    if [TestForError $reply] {
        return
    }
    destroy $root
    $updater invoke
}

