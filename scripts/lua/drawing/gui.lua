-----------------------
-- Drawing fucntions --

function gui_drawtext(control, menu, ishover)
	button.w, button.h = drawtext(control.text, control.x, control.y, 1)
end

function gui_drawbutton(control, menu, ishover)
	button.w, button.h = drawtext(control.text, control.x, control.y, 1)
end

function gui_drawcheckbox(control, menu, ishover)
	local w, h
	w, h = settexture((control.var ~= 0 and "data/hud/default/icons/checkbox_off.png") or "data/hud/default/icons/checkbox_on.png")
	drawquad(control.x, control.y, 1, 1, 1)
	button.w, button.h = w, h
	w, h = drawtext(control.text, control.x, control.y, 1)
	button.w, button.h = button.w+w, button.h+h
end

function gui_drawradio(control, menu, ishover)
	local w, h
	w, h = settexture((control.var ~= control.val and "data/hud/default/icons/radio_off.png") or "data/hud/default/icons/radio_on.png")
	drawquad(control.x, control.y, 1, 1, 1)
	button.w, button.h = w, h
	w, h = drawtext(control.text, control.x, control.y, 1)
	button.w, button.h = button.w+w, button.h+h
end

---------------------------
-- New control functions --

function gui_menuclick(control, state)
	if (state == 1) then control.isdown = true
	else control.isdown = false end
end

function gui_newmenu(title, x, y)
	local t = { text = title, x = x, y = y, w = 0, h = 0, mousedown = false, onevent = gui_menuclick, draw = gui_drawbutton, callback = gui_callback }
	return t
end

function gui_newtext(text, x, y)
	local t = { text = text, x = x, y = y, w = 0, h = 0, draw = gui_drawtext }
	return t
end

function gui_newbutton(text, x, y, onclick)
	local t = { text = text, x = x, y = y, w = 0, h = 0, onevent = onclick, draw = gui_drawbutton, callback = gui_callback }
	return t
end

function gui_newcheckbox(var, text, x, y, onchange)
	local t = { var = var, text = text, x = x, y = y, w = 0, h = 0, onevent = onchange, draw = gui_drawcheckbox, callback = gui_callback }
	return t
end

function gui_newradio(var, value, text, x, y, onchange)
	local t = { var = var, value = value, text = text, x = x, y = y, w = 0, h = 0, onevent = onchange, draw = gui_drawradio, callback = gui_callback }
	return t
end

-------------------------
-- Auxiliary functions --

-- radio onevent function
function gui_change(control, state)
	control.var = control.value
end

-- checkbox onevent function
function gui_toggle(control, state)
	if (control.var ~= 0) then control.var = 0
	else control.var = 1 end
end

-- 3-way checkbox onevent function
function gui_toggle3(control, state)
	if (control.var == 0) then control.var = 1
	elseif (control.var == 1) then control.var = 2
	else control.var = 0 end
end

-- checks whether the point is inside the control's bounding box
function gui_inrect(px, py, control)
	return (px >= control.x and px<(control.w+control.x)) and (py >= control.y and py<(control.h+control.y))
end

-- get the item under the mouse
function gui_gethover(menu, x, y)
	local i
	for i = #menu, 1, -1 do
		if (gui_inrect(x, y, menu[v])) then return menu[v] end
	end
end

-- handle a mouse click
function gui_handleclick(menu, hover, button, state)
	if (button == 1) then
		if (type(control.onevent) == "function") then control:onevent(state)
		elseif (type(onchange) == "string") then load(control.onevent)() end
	end
end

-------------------------
-- Main loop functions --

-- draw all controls on screen
function gui_drawmenu(menu, hover)
	for i,v in ipairs(menu) do
		v:draw(menu, (hover == v))
	end
end

-- check for updates from C++
function gui_update()
	
end

---------------
-- Test menu --

mainmenu = {
	gui_newmenu("main menu", 0.25, 0.25),
	gui_newtext("main menu", 0.27, 0.27),
	gui_newbutton("new game", 0.29, 0.29, ""),
	gui_newcheckbox(game.zoom, "toggle zoom", 0.31, 0.31, gui_toggle),
	gui_newradio(game.floatspeed, 500, "fs 500", 0.33, 0.33, gui_change),
}

curmenu = mainmenu
