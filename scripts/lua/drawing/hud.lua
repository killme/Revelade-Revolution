-- draw the entire HUD
function gameplayhud()
	-- draw frame
	settexture("data/hud/Frame.png")
	drawquad(0.004, 0.01, 1, 1, 2.6)
	
	-- draw bars
	settexture("data/hud/EmptyBar.png")
	drawquad(0.07, 0.1,   1, 1, 2.4)
	drawquad(0.07, 0.124, 1, 1, 2.4)
	drawquad(0.07, 0.148, 1, 1, 2.4)
	
	settexture("data/hud/RedBar.png")
	drawquad(0.07, 0.1,   math.min(player1.health/130, 1), 1, 2.4)
	settexture("data/hud/BlueBar.png")
	drawquad(0.07, 0.124, math.min(player1.mana/130, 1), 1, 2.4)
	settexture("data/hud/GreenBar.png")
	drawquad(0.07, 0.148, math.min(player1.experience/100, 1), 1, 2.4)
	
	-- draw avatar
	
	
	-- draw minimap
	drawminimap(1, 0, 0.8, "data/hud/radar.png", "data/hud/compass.png")
	
	-- display fps
	drawtext("fps " .. tostring(fps), 0.87, 0.94, 1)
end
