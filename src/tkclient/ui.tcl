######################################################################
# tk_client.ui
#
# All user interface definitions for the client.
#
# Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
# 

#
# $Header: /home/ncvs/src/tkclient/ui.tcl,v 1.1 1997/02/13 17:04:56 dom Exp $
#
# $Log: ui.tcl,v $
# Revision 1.1  1997/02/13 17:04:56  dom
# Initial revision
#
# Revision 1.0  1996/05/14 08:43:13  dom
# Initial revision
#
#

proc tkclient_ui {} {
    
    ## Widget definitions

    # Frames

    frame .bt

    frame .af

    frame .m \
            -relief raised \
            -bd 2

    # Menus

    # File 
    set m .m.file
    menubutton $m \
            -text "File" \
            -menu $m.m \
            -underline 0

    menu $m.m \
            -tearoff false
    $m.m add command \
            -label "Connect" \
            -underline 0 \
            -command MFileConnect
    $m.m add separator
    $m.m add command \
            -label "Exit" \
            -underline 1 \
            -command MFileExit

    # Options
    set m .m.opt
    menubutton $m \
            -text "Options" \
            -menu $m.m \
            -underline 0
    menu $m.m \
            -tearoff false
    $m.m add command \
            -label "Host Details..." \
            -underline 0 \
            -command DlgHostDetails
    global AcceptPages
    $m.m add checkbutton \
            -label "Accept Pages" \
            -underline 0 \
            -variable AcceptPages \
            -command MOptionAcceptPages
    $m.m add separator
    $m.m add command \
            -label "Save Options" \
            -underline 0 \
            -command MOptionSave

    # Help
    set m .m.help
    menubutton $m \
            -text "Help" \
            -menu $m.m \
            -underline 0

    menu $m.m \
            -tearoff false
    $m.m add command \
            -label "About" \
            -underline 0 \
            -command MHelpAbout

    unset m

    # Areas

    listbox .af.area_list \
            -height 10 \
            -relief ridge \
            -width 30 \
            -yscrollcommand ".af.area_scroll set"

    label .af.area_label \
            -width 30 \
            -height 1 \
            -text "Not Connected"

    scrollbar .af.area_scroll \
            -activerelief sunken \
            -command ".af.area_list yview" \
            -orient v

    # Buttons
    button .bt.mesg \
            -padx 9 \
            -pady 3 \
            -text {Messages} \
            -command BMessages
    button .bt.talk \
            -padx 9 \
            -pady 3 \
            -text Talk \
            -command BTalk
    button .bt.who \
            -padx 9 \
            -pady 3 \
            -text Who \
            -command BWho
    button .bt.page \
            -padx 9 \
            -pady 3 \
            -text Page \
            -command BPage
    button .bt.fortune \
            -padx 9 \
            -pady 3 \
            -text Fortune \
            -command BFortune

    ## Geometry management

    # Frames

    pack .m  \
            -anchor n \
            -fill x
    pack .bt \
            -anchor e \
            -side right \
            -fill y
    pack .af \
            -anchor w \
            -expand true \
            -side left \
            -fill both

    # Menu

    pack .m.file \
            -side left
    pack .m.opt \
            -side left
    pack .m.help \
            -side right

    # Buttons

    pack .bt.mesg \
            -anchor n \
            -fill x
    pack .bt.talk \
            -anchor n \
            -fill x
    pack .bt.who \
            -anchor n \
            -fill x
    pack .bt.page \
            -anchor n \
            -fill x
    pack .bt.fortune \
            -anchor n \
            -fill x

    # Areas

    pack .af.area_label \
            -side top \
            -anchor nw \
            -fill x
    pack .af.area_list \
            -side left \
            -expand true \
            -fill both
    pack .af.area_scroll \
            -side right \
            -fill y

    # Set up a default entry in the listbox
    global InitHandlers AfAreaState
    set InitHandlers(Area) {
        .af.area_list insert end "Please set host options and select"
        .af.area_list insert end "\"Connect\" from the File menu"
        set AfAreaState none
    }
}

######################################################################
# DlgHostDetails
#
# Shows the current options and gives the user a chance to save them.
#
proc DlgHostDetails {} {
    if [winfo exists .dlg] {
        return
    }

    # Globals access

    foreach i {LoginName Password SpiderHost SpiderPort} {
        global $i
        global old$i
        set old$i [set $i]
    }

    # Frames

    toplevel .dlg
    wm title .dlg "Host Details"
    frame .dlg.opt
    frame .dlg.bt

    # Options

    foreach i {1 2 3 4} {
        frame .dlg.opt.$i
        label .dlg.opt.$i.lbl
        entry .dlg.opt.$i.ent -width 20
    }
    .dlg.opt.1.lbl configure -text "Username:"
    .dlg.opt.2.lbl configure -text "Password:"
    .dlg.opt.3.lbl configure -text "Host:"
    .dlg.opt.4.lbl configure -text "Port:"
    .dlg.opt.1.ent configure -textvariable LoginName
    .dlg.opt.2.ent configure -textvariable Password -show "*"
    .dlg.opt.3.ent configure -textvariable SpiderHost
    .dlg.opt.4.ent configure -textvariable SpiderPort

    # Buttons

    button .dlg.bt.ok -text OK -command {
        destroy .dlg
    }
    button .dlg.bt.can -text Cancel -command {
        foreach i {LoginName Password SpiderHost SpiderPort} {
            set $i [set old$i]
        }
        destroy .dlg
    }

    ## Pack the widgets

    pack .dlg.opt -expand true -fill both
    pack .dlg.bt -fill x
    foreach i {1 2 3 4} {
        pack .dlg.opt.$i -fill x -expand true
        pack .dlg.opt.$i.ent -side right
        pack .dlg.opt.$i.lbl -side left
    }
    foreach i {ok can} {pack .dlg.bt.$i -side left -fill x -expand true}
}

######################################################################
# DlgPage
#
# Shows a page that's just arrived in.
#
proc DlgPage {mesg} {
    set name [lindex $mesg 0]
    set text [lindex $mesg 1]
    tk_dialog .page "Pager" "Page from $name:\n$text" info 0 OK
}

######################################################################
# DlgPageTo
#
# Sets up a dialog box to send off a pager message.
#
proc DlgPageTo {who} {
    # Frames
    toplevel .pageto
    frame .pageto.var
    frame .pageto.bt
    frame .pageto.var.1

    # Options
    set f .pageto.var
    label $f.1.lbl \
            -text "To:" \
            -anchor w
    entry $f.1.ent \
            -width 20
    entry $f.text

    # Buttons

    button .pageto.bt.ok -text Send -command {
        set who [.pageto.var.1.ent get]
        set text [.pageto.var.text get]
        DoCmd [list "PAGE TO $who" "$text"]
        destroy .pageto
    }
    button .pageto.bt.can -text Cancel -command {
        destroy .pageto
    }

    ## Pack the widgets

    pack .pageto.var -expand 1 -fill both
    pack .pageto.bt -fill x -pady 2

    pack .pageto.var.1 -expand 1 -fill x -pady 2
    pack .pageto.var.1.lbl -side left
    pack .pageto.var.1.ent -side left -expand 1 -anchor w
    pack .pageto.var.text -expand 1 -fill x -pady 2

    pack .pageto.bt.ok -side left -expand 1
    pack .pageto.bt.can -side left -expand 1

    # Insert the value that we came with.
    .pageto.var.1.ent insert end "$who"
}
