#include "gui_list_item_entry.h"


void GuiListItemEntry::draw(bool clearBeforeDraw, bool mustUpdateScreen, bool highlight)
{
	char buffer[2048];
	int yy = y;
	int initialY = y;
	int imageX = 1;
	int imageWidth = PLOP_THUMBNAIL_MAX_WIDTH;
	int textX = imageX + imageWidth + 4;

	if (clearBeforeDraw && !highlight) {
		FillArea(x, initialY, screenWidth, getHeight(), WHITE);
	}

	if (highlight) {
		FillArea(x, initialY - 1, screenWidth, getHeight(), LGRAY);
	}

	if (_hasEntry == false) {
		if (mustUpdateScreen) {
			PartialUpdate(x, initialY, screenWidth, getHeight());
		}
		return;
	}

	if (entry.preview_picture_path != "" && entry.preview_picture_type == 1) {
		// Thumbnails are always stored as PNG
		ibitmap *img = LoadPNG(entry.preview_picture_path.c_str(), 1);
		if (img != NULL) {
			DrawBitmap(imageX, yy + 1, img);
			free(img);
		}
	}

	SetFont(titleFont, entry.is_empty ? DGRAY : BLACK);
	snprintf(buffer, sizeof(buffer), "%s", entry.title.c_str());
	DrawString(textX, yy, buffer);
	yy += titleFont->height;


	SetFont(infosFont, entry.is_empty ? LGRAY : DGRAY);
	snprintf(buffer, sizeof(buffer), "#%d/%s a%d/%d *%d/%d u%d/%d t%d %s", entry.id, entry.remote_id.c_str(), entry.local_is_archived, entry.remote_is_archived, entry.local_is_starred, entry.remote_is_starred, entry.local_updated_at, entry.remote_updated_at, (int)ceil((float)entry.reading_time/(float)60), (entry.local_content_file_epub != "" ? "E" : "H"));
	DrawString(textX, yy, buffer);
	yy += infosFont->height;

	SetFont(infosFont, entry.is_empty ? LGRAY : DGRAY);
	snprintf(buffer, sizeof(buffer), "%s", entry.url.c_str());
	DrawString(textX, yy, buffer);
	yy += infosFont->height;

	if (mustUpdateScreen) {
		updateScreen();
	}
}

void GuiListItemEntry::updateScreen()
{
	PartialUpdate(x, y, screenWidth, getHeight()-1);
}

