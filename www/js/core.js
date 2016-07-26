function CheckMenu(idToCheck, newID)
{
	if ($("#" + idToCheck).hasClass("menu-item-selected"))
	{
		$("#sub" + idToCheck).slideUp("fast", function() {
			$("#" + idToCheck).removeClass("menu-item-selected");
			
			if (idToCheck != newID)
			{
				$("#" + newID).toggleClass("menu-item-selected");
				$("#sub" + newID).slideDown("fast");
			}
		});
		return true;
	}
	return false;
}

function CheckSubmenu(idToCheck, newID)
{
	if ($("#" + idToCheck).hasClass("submenu-item-selected"))
	{
		if (idToCheck != newID)
		{
			$("#" + idToCheck).removeClass("submenu-item-selected");
			$("#" + newID).toggleClass("submenu-item-selected");
		}
		return true;
	}
	return false;
}

function MenuClick(id)
{
	var status = false;
	
	if(!status && CheckMenu("menu-item-player", id))
		status = true;
	if(!status && CheckMenu("menu-item-banned", id))
		status = true;
	if(!status && CheckMenu("menu-item-options", id))
		status = true;
		
	if(!status)
	{
		
		$("#" + id).toggleClass("menu-item-selected");
		$("#sub" + id).slideDown("fast");
	}
}

function SubmenuClick(id)
{
	var status = false;
	
	if(!status && CheckSubmenu("submenu-item-players-on", id))
		status = true;
	if(!status && CheckSubmenu("submenu-item-all-players", id))
		status = true;
	if(!status && CheckSubmenu("submenu-item-banned-account", id))
		status = true;
	if(!status && CheckSubmenu("submenu-item-banned-ip", id))
		status = true;
	if(!status && CheckSubmenu("submenu-item-banned-hwid", id))
		status = true;
		
	if(!status)
		$("#" + id).toggleClass("submenu-item-selected");
}