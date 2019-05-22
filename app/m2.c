
/*
 *
 *
 * Menu2 v2.0
 * Martin Hubacek
 * 18.3.2013
 * http://martinhubacek.cz
 *
 *
 */


#include "m2.h"

int keyPress = 0;

int menu2_init(Menu *menu)
{
        	//
	// Get number of items in menu, search for the first NULL
	//
	MenuItem **iList = menu->items;
	menu->len = 0;
	for (; *iList != 0; ++iList)
	  menu->len++;

    menu->menu_next = NULL;
    menu->menu_previous = NULL;

/*
    bc_module_lcd_draw_string(10, 10, "xxx", 1);

    bc_module_lcd_update();*/

	// Functional :)
	// menuItem, menuTopPos, cursorTopPos
	if(menu->selectedIndex != -1)
	{
	  // If item on the first screen
	  if(menu->selectedIndex < MENU_LINES)
	  {
		  menu->menuItem = menu->selectedIndex;
		  menu->cursorTopPos = menu->selectedIndex;
		  menu->menuTopPos = 0;
	  } else {
		 // Item is on other screen
		  menu->menuItem = menu->selectedIndex;
		  menu->cursorTopPos = MENU_LINES - 1;
		  menu->menuTopPos = menu->selectedIndex - menu->cursorTopPos;
	  }
	}

    return 0;
}

int menu2_event(Menu *menu, uint8_t keyPress)
{
    while(menu->menu_next)
    {
        menu = menu->menu_next;
    }


		  //
		  // Down
		  //
		if(keyPress == BTN_DOWN)
		  {
			if(menu->menuItem != menu->len-1)
			{
				// move to next item
				menu->menuItem++;

				if(menu->cursorTopPos >= MENU_LINES-1 ||
						(menu->cursorTopPos ==/**/ ((MENU_LINES/**/)/2) && ((menu->len) - menu->menuItem  ) > ((MENU_LINES-1/**/)/2)) )
				  menu->menuTopPos++;
				else
				  menu->cursorTopPos++;

			} else {
				// Last item in menu => go to first item
				menu->menuItem = 0;
				menu->cursorTopPos = 0;
				menu->menuTopPos = 0;
			}
		  }

		  //
		  // Up
		  //
		 if(keyPress == BTN_UP)
		  {
			if(menu->menuItem != 0)
			{
			menu->menuItem--;

			if(menu->cursorTopPos > 0 &&
					!((menu->cursorTopPos == MENU_LINES/2) && (menu->menuItem >= MENU_LINES/2)))
			  menu->cursorTopPos--;
			else
			  menu->menuTopPos--;
			  } else {
				// go to the last item in menu
				menu->menuItem = menu->len-1;

				if(menu->len <= MENU_LINES)
				{
					menu->menuTopPos = 0;
				} else {
					menu->menuTopPos = menu->menuItem;
				}
				if(menu->menuTopPos > menu->len - MENU_LINES && menu->len >= MENU_LINES)
				{
					menu->menuTopPos = menu->len - MENU_LINES;
				}

				menu->cursorTopPos = menu->menuItem - menu->menuTopPos;
			  }
		  }


		  //
		  // Enter
		  //
		 if(keyPress == BTN_ENTER || keyPress == BTN_RIGHT)
		  {
			keyPress = 0;
			menu->selectedIndex = menu->menuItem;
			int flags = menu->items[menu->selectedIndex]->flags;

			// checkbox
			if(flags & MENU_ITEM_IS_CHECKBOX)
			{
				menu->items[menu->selectedIndex]->flags ^= MENU_ITEM_IS_CHECKED;
				// Force refersh
				menu->lastMenuItem = -1;
			}

            MenuItem *item = menu->items[menu->selectedIndex];
            menu->callback(menu, item);

            if (item->submenu)
            {
                Menu *next = menu->menu_next = item->submenu;
                next->menu_previous = menu;
            }
            else
            {
                // Move back to previous parent menu
                Menu *prev = menu->menu_previous;
                prev->menu_next = NULL;
            }

/*
			// Item is submenu - parameter in callback
			if(flags & MENU_CALLBACK_IS_SUBMENU && menu->items[menu->selectedIndex]->callback)
			{
				menu2((Menu*)menu->items[menu->selectedIndex]->callback);
				// Force refersh
				menu->lastMenuItem = -1;
			}

			// callback
			if(flags & MENU_CALLBACK_IS_FUNCTION && menu->items[menu->selectedIndex]->callback)
			{
				(*menu->items[menu->selectedIndex]->callback)(menu);
				// Force refersh
				menu->lastMenuItem = -1;
			}

			// normal item, so exit
			if((menu->items[menu->selectedIndex]->callback == 0) && ((flags & MENU_ITEM_IS_CHECKBOX) == 0))
			{
				return menu->menuItem;
			}*/

		  }


		  //
		  // Left - back
		  //
		  if(keyPress == BTN_LEFT)
		  {
			keyPress = 0;
			menu->selectedIndex = 0;

            if (menu->menu_previous)
            {
                Menu *p = menu->menu_previous;
                p->menu_next = NULL;
            }

					return -2;
		  }


}

int menu2_draw(Menu *menu)
{

    while(menu->menu_next)
    {
        menu = menu->menu_next;
    }

	int i = 0;
    //displayClear();
    {

		//
		// If menu item changed -> refresh screen
		//
		//if(menu->lastMenuItem != menu->menuItem || (menu->refresh && MENU_MS_TICK > menu->refreshTimer))
		{
			/*if(menu->refresh)
				 menu->refreshTimer = MENU_MS_TICK + menu->refresh;*/

		  displayClear();
		  displayString(menu->title[menuLanguage],0,0);

		  // Menu debug
		  //sprintf(buffer, "%d,%d,%d", menuItem, menuTopPos, cursorTopPos);
		  //displayString(buffer,0,0);

		  i = 0;
		  while((i + menu->menuTopPos) < menu->len &&   i < MENU_LINES)
		  {
			int index = menu->menuTopPos + i;
			if(menu->menuItem == index && MENU_LINES > 1)
				displayString(ARROW_SYMBOL, 0, ROW(i+1));

			int posx = strlen(menu->items[index]->text[menuLanguage]) + 4;

			if(MENU_LINES > 1)
				displayString(menu->items[index]->text[menuLanguage], COL(1+ARROW_GAP), ROW(i+1));
			else
				displayString(menu->items[index]->text[menuLanguage], COL(0), ROW(i+1));

            if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_NUMBER)
                displayNumber(*((int*)menu->items[index]->parameter), COL(posx), ROW(i+1));

            if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_STRING)
                lcdBufferString((char*)(menu->items[index]->parameter), COL(posx + 1),ROW(i+1));

            if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX) {
                if(menu->items[index]->flags & MENU_ITEM_IS_CHECKED)
                    displayString("X", COL(1),ROW(i+1));
                else
                    displayString("O", COL(1),ROW(i+1));
            }

			i++;
		  }

			#ifdef displayDraw
			  displayDraw();
			#endif

		  menu->lastMenuItem = menu->menuItem;
		}


	}

	return -1;

}
