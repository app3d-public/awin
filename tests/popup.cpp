#include <awin/popup.hpp>

void test_popup()
{
    awin::popup::message_box("Message", "Title");
    awin::popup::confirm_message_box("Message", "Title");
    acul::vector<awin::popup::FilePattern> pattern{{"All files", {"*.*"}}};
    assert(!awin::popup::open_file_dialog("Title", pattern).empty());
    assert(!awin::popup::save_file_dialog("Title", pattern).empty());
    assert(!awin::popup::open_folder_dialog("Title").empty());
}