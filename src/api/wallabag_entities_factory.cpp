#include "wallabag_entities_factory.h"


time_t WallabagEntitiesFactory::datetimeStringToTimeT(const char *str)
{
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	strptime(str, "%Y-%m-%dT%H:%M:%S%z", &tm);
	return timegm(&tm);
}


Entry WallabagEntitiesFactory::createEntryFromJson(json_object *item)
{
	int id = json_object_get_int(json_object_object_get(item, "id"));
	int is_archived = json_object_get_int(json_object_object_get(item, "is_archived"));
	int is_starred = json_object_get_int(json_object_object_get(item, "is_starred"));
	const char *title = json_object_get_string(json_object_object_get(item, "title"));
	const char *url = json_object_get_string(json_object_object_get(item, "url"));
	const char *content = json_object_get_string(json_object_object_get(item, "content"));
	const char *created_at_str = json_object_get_string(json_object_object_get(item, "created_at"));
	const char *updated_at_str = json_object_get_string(json_object_object_get(item, "updated_at"));
	int reading_time = json_object_get_int(json_object_object_get(item, "reading_time")) * 60;
	const char *preview_picture = json_object_get_string(json_object_object_get(item, "preview_picture"));

	time_t created_at_ts = datetimeStringToTimeT(created_at_str);
	time_t updated_at_ts = datetimeStringToTimeT(updated_at_str);

	Entry entry;
	entry.id = 0;

	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%d", id);
	entry.remote_id = tmp;

	// When creating an entry locally, its local values are the same as remote ones
	entry.local_is_archived = is_archived;
	entry.remote_is_archived = is_archived;

	entry.local_is_starred = is_starred;
	entry.remote_is_starred = is_starred;

	entry.title = title != NULL ? title : std::string();
	entry.url = url != NULL ? url : std::string();
	entry.content = content != NULL ? content : std::string();

	entry.local_created_at = created_at_ts;
	entry.remote_created_at = created_at_ts;

	entry.local_updated_at = updated_at_ts;
	entry.remote_updated_at = updated_at_ts;

	entry.reading_time = reading_time;
	entry.preview_picture_url = preview_picture != NULL ? preview_picture : std::string();
	entry.local_content_file_html = std::string();
	entry.local_content_file_epub = std::string();

	if (entry.title == "No title found" && entry.reading_time <= 0) {
		entry.is_empty = true;
	} else {
		entry.is_empty = false;
	}

	return entry;
}


Entry WallabagEntitiesFactory::mergeLocalAndRemoteEntries(Entry &local, Entry &remote)
{
	Entry entry;
	entry._isChanged = false;
	bool isLocalUpdated = false;

	entry.id = local.id;
	if (local.remote_id != remote.remote_id) {
		entry.remote_id = remote.remote_id;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.remote_id = local.remote_id;
	}

	if (local.remote_updated_at != remote.remote_updated_at) {
		// Note: this has no effect on isLocalUpdated
		entry.remote_updated_at = remote.remote_updated_at;
		entry._isChanged = true;
	}
	else {
		entry.remote_updated_at = local.remote_updated_at;
	}

	if (remote.remote_updated_at > local.local_updated_at && local.local_is_archived != remote.remote_is_archived) {
		entry.local_is_archived = remote.remote_is_archived;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.local_is_archived = local.local_is_archived;
	}

	if (local.remote_is_archived != remote.remote_is_archived) {
		entry.remote_is_archived = remote.remote_is_archived;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.remote_is_archived = local.remote_is_archived;
	}

	if (remote.remote_updated_at > local.local_updated_at && local.local_is_starred != remote.remote_is_starred) {
		entry.local_is_starred = remote.remote_is_starred;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.local_is_starred = local.local_is_starred;
	}

	if (local.remote_is_starred != remote.remote_is_starred) {
		entry.remote_is_starred = remote.remote_is_starred;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.remote_is_starred = local.remote_is_starred;
	}

	// For some fields, we always use what comes from the remote
	if (local.remote_created_at != remote.remote_created_at) {
		entry.remote_created_at = remote.remote_created_at;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.remote_created_at = local.remote_created_at;
	}

	if (local.title != remote.title) {
		entry.title = remote.title;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.title = local.title;
	}

	if (local.url != remote.url) {
		entry.url = remote.url;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.url = local.url;
	}

	if (local.content != remote.content) {
		entry.content = remote.content;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.content = local.content;
	}

	if (local.reading_time != remote.reading_time) {
		entry.reading_time = remote.reading_time;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.reading_time = local.reading_time;
	}

	if (local.preview_picture_url != remote.preview_picture_url) {
		entry.preview_picture_url = remote.preview_picture_url;
		isLocalUpdated = true;
		entry._isChanged = true;
	}
	else {
		entry.preview_picture_url = local.preview_picture_url;
	}

	// For some other fields that are more specific to the application, we always use what we had locally
	entry.local_created_at = local.local_created_at;
	entry.local_content_file_html = local.local_content_file_html;
	entry.local_content_file_epub = local.local_content_file_epub;

	if (isLocalUpdated && remote.remote_updated_at > local.local_updated_at) {
		// Some new data have been obtained from the server,
		// AND the entry has been updated more recently on the server than on the device
		// => use the updated_at date from the server
		entry.local_updated_at = remote.remote_updated_at;
		entry._isChanged = true;
	}
	else {
		// Entry has not been updated on the server, or it's more recent on the ereader
		// => Keep the updated_at date we had on the device
		entry.local_updated_at = local.local_updated_at;
	}

	return entry;
}
