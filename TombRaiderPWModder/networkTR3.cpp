/*
int test() {
    // We need a class for hanling map download link or local file.
    // Checking files checksum and or remove them
    // Change to the game's directory
    //chdir("/.");

    // List of files/directories to be removed
    const char* filesToRemove[] = {
        "./audio/cdaudio.wad",
        "./installscript.vdf",
        "./D3DImm.dll",
        "./README",
        "./DEC130.DLL",
        "./pix/CREDIT06.bmp",
        "./pix/CREDIT02.bmp",
        "./pix/Shivaville.bmp",
        "./pix/CREDIT04.bmp",
        "./pix/CREDIT01.BMP",
        "./pix/CREDIT08.bmp",
        "./pix/TEMPLE.bmp",
        "./pix/THEEND.BMP",
        "./pix/HOUSE.BMP",
        "./pix/TITLEUK.bmp",
        "./pix/CULT.BMP",
        "./pix/CREDIT05.bmp",
        "./pix/LEGAL.BMP",
        "./pix/CREDIT09.bmp",
        "./pix/JUNGLE.bmp",
        "./pix/CREDIT07.bmp",
        "./pix/CREDIT03.bmp",
        "./pix/THEEND2.BMP",
        "./WINPLAY.DLL",
        "./tomb3_ConfigTool.exe",
        "./Uninst.isu",
        "./tomb3_ConfigTool.json",
        "./data.bin",
        "./data/MAIN.SFX",
        "./data/tombpc.dat",
        "./data/HOUSE.tr2",
        "./data/Level3.tr2",
        "./data/TITLE.TR2",
        "./data/Level4.tr2",
        "./data/Test.tr2",
        "./data/Level1.tr2",
        "./data/Level2.tr2",
        "./ReadMe.pdf",
        "./saves",
        "./tomb3.exe",
        "./dgVoodoo.conf",
        "./EDEC.DLL",
        "./DDraw.dll",
        "./WINSTR.DLL",
        "./ExtraOptions/GUI scale/InvGuiScale_0_5.reg",
        "./ExtraOptions/GUI scale/GameGuiScale_2_0.reg",
        "./ExtraOptions/GUI scale/InvGuiScale_1_5.reg",
        "./ExtraOptions/GUI scale/GameGuiScale_1_0.reg",
        "./ExtraOptions/GUI scale/GameGuiScale_0_5.reg",
        "./ExtraOptions/GUI scale/GameGuiScale_1_5.reg",
        "./ExtraOptions/GUI scale/InvGuiScale_2_0.reg",
        "./ExtraOptions/GUI scale/InvGuiScale_0_7.reg",
        "./ExtraOptions/GUI scale/InvGuiScale_1_0.reg",
        "./ExtraOptions/GUI scale/GameGuiScale_0_7.reg",
        "./ExtraOptions/Sound/UnderwaterMusicMute_Full.reg",
        "./ExtraOptions/Sound/UnderwaterMusicMute_Quiet.reg",
        "./ExtraOptions/Sound/InventoryMusicMute_None.reg",
        "./ExtraOptions/Sound/InventoryMusicMute_Full.reg",
        "./ExtraOptions/Sound/InventoryMusicMute_Quiet.reg",
        "./ExtraOptions/Sound/UnderwaterMusicMute_None.reg",
        "./ExtraOptions/Effects/Footprints_OFF.reg",
        "./ExtraOptions/Effects/improved lasers_OFF.reg",
        "./ExtraOptions/Effects/Sophia Rings_PSX.reg",
        "./ExtraOptions/Effects/Sophia Rings_ImprovedPC.reg",
        "./ExtraOptions/Effects/improved rain_ON.reg",
        "./ExtraOptions/Effects/improved lasers_ON.reg",
        "./ExtraOptions/Effects/Sophia Rings_PC.reg",
        "./ExtraOptions/Effects/upv_wake_ON.reg",
        "./ExtraOptions/Effects/underwater dust_OFF.reg",
        "./ExtraOptions/Effects/Footprints_ON.reg",
        "./ExtraOptions/Effects/kayak_mist_OFF.reg",
        "./ExtraOptions/Effects/improved rain_OFF.reg",
        "./ExtraOptions/Effects/underwater dust_ON.reg",
        "./ExtraOptions/Effects/kayak_mist_ON.reg",
        "./ExtraOptions/Effects/upv_wake_OFF.reg",
        "./ExtraOptions/View/psx_fov_OFF.reg",
        "./ExtraOptions/View/psx_fov_ON.reg",
        "./ExtraOptions/reset all options.reg",
        "./ExtraOptions/Lara's moveset/flexible_crawl_OFF.reg",
        "./ExtraOptions/Lara's moveset/flexible_sprint_OFF.reg",
        "./ExtraOptions/Lara's moveset/flexible_crawl_ON.reg",
        "./ExtraOptions/Lara's moveset/duck_roll_ON.reg",
        "./ExtraOptions/Lara's moveset/slide_to_run_ON.reg",
        "./ExtraOptions/Lara's moveset/crawl_tilt_ON.reg",
        "./ExtraOptions/Lara's moveset/duck_roll_OFF.reg",
        "./ExtraOptions/Lara's moveset/flexible_sprint_ON.reg",
        "./ExtraOptions/Lara's moveset/slide_to_run_OFF.reg",
        "./ExtraOptions/Lara's moveset/crawl_tilt_OFF.reg",
        "./ExtraOptions/misc/disable_colorkey_OFF.reg",
        "./ExtraOptions/misc/psx_water_color_ON.reg",
        "./ExtraOptions/misc/disable_colorkey_ON.reg",
        "./ExtraOptions/misc/dozy_ON.reg",
        "./ExtraOptions/misc/dozy_OFF.reg",
        "./ExtraOptions/misc/psx_crystal_sfx_OFF.reg",
        "./ExtraOptions/misc/disable_gamma_OFF.reg",
        "./ExtraOptions/misc/psx_water_color_OFF.reg",
        "./ExtraOptions/misc/disable_gamma_ON.reg",
        "./ExtraOptions/misc/psx_crystal_sfx_ON.reg",
        "./ExtraOptions/GUI Style/psx_boxes_ON.reg",
        "./ExtraOptions/GUI Style/ammo_counter_PC.reg",
        "./ExtraOptions/GUI Style/PickupDisplay_OFF.reg",
        "./ExtraOptions/GUI Style/bar_positions_Improved.reg",
        "./ExtraOptions/GUI Style/improved_poison_bar_OFF.reg",
        "./ExtraOptions/GUI Style/PickupDisplay_ON.reg",
        "./ExtraOptions/GUI Style/Shadow_PSX.reg",
        "./ExtraOptions/GUI Style/Bars_PC.reg",
        "./ExtraOptions/GUI Style/psx_boxes_OFF.reg",
        "./ExtraOptions/GUI Style/ammo_counter_PSX.reg",
        "./ExtraOptions/GUI Style/bar_positions_PSX.reg",
        "./ExtraOptions/GUI Style/psx_text_colors_OFF.reg",
        "./ExtraOptions/GUI Style/Bars_PSX.reg",
        "./ExtraOptions/GUI Style/psx_text_colors_ON.reg",
        "./ExtraOptions/GUI Style/bar_positions_PC.reg",
        "./ExtraOptions/GUI Style/Shadow_PC.reg",
        "./ExtraOptions/GUI Style/improved_poison_bar_ON.reg",
        "./WINSDEC.DLL",
        "./ReadmeHost.txt",
        "Jonson-TheInfadaCult.zip"
    };
    const char* directoriesToRemove[] = {
        "./data",
        "./audio",
        "./pix",
        "./ExtraOptions/misc",
        "./ExtraOptions/Sound",
        "./ExtraOptions/Lara's moveset",
        "./ExtraOptions/Effects",
        "./ExtraOptions/GUI Style",
        "./ExtraOptions/GUI scale",
        "./ExtraOptions/View",
        "./ExtraOptions"
    };

    // Remove files and directories
    for (const char* item : filesToRemove)
    {
        remove(item);
    }
    for (const char* item : directoriesToRemove)
    {
        rmdir(item);
    }

    // Download file
    const char* url = "https://www.trle.net/levels/levels/2023/1123/Jonson-TheInfadaCult.zip";
    const char* filename = "Jonson-TheInfadaCult.zip";

    if (!downloadFile(url, filename))
    {
        // Check MD5sum
        const char* expectedMD5 = "152d33e5c28d7db6458975a5e53a3122";
        std::string md5sum = calculateMD5(filename);
        std::cout << md5sum << std::endl;

        if (md5sum == expectedMD5)
        {
            // Extract the ZIP file
            const char* extractPath = "./";
            extractZip(filename, extractPath);

        }
        else
        {
            std::cerr << "Download problem" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cerr << "Download problem" << std::endl;
        return 1;
    }

    // Clean up downloaded file
    remove(filename);

    return 0;
}
*/
