// (C)2004 S2 Games
// c_uixmlparser.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "shared_common.h"

#include "c_uixmlparser.h"
//=============================================================================

/***

<?xml version="1.0" encoding="UTF-8" ?>

<interface name="main_menu">
	<panel name="test_panel"
		x="10"
		y="10"
		width="50"
		height="50"
		image="blah.jpg"
		over_image="blah_over.jpg"
		down_image="blah_down.jpg"
		disabled_image="blah_disabled.jpg"
		disabled="true"
	>
		<button name="button1">
		</button>
	</panel>

	<panel name="parent_panel">
		<panel name="child_panel">
		</panel>
	</panel>
</interface>

***/
