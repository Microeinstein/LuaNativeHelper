process = {
	nativehelp = "winNativeHelper",
	libraryExt = "dll"
}

local lib64 = string.format("%s64.%s", process.nativehelp, process.libraryExt)
local lib32 = string.format("%s32.%s", process.nativehelp, process.libraryExt)
local libinit = package.loadlib(lib64, "libinit") or package.loadlib(lib32, "libinit")
if libinit then
	libinit()
end

nativeHelper.clearAll()
print("Screen cleared successfully")
print("Wait a sec...")
