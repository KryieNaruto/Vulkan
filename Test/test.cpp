#include <ThirdPartyManager/third_party_manager_global.h>
#include <Editor/editor_include.h>
int main(int argc, char* argv[]) {

	ThirdParty::init();
	
	Editor::loadEditors();
	Editor::editor->onRender();
	
	Core::destroy();
	ThirdParty::destroy();
	return 0;
}