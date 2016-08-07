function globals_init
    %% DEBUG AND OUTPUT VARIABLES-----------------------------------------------------------------%%
    % Debug Controls
    global DEBUG_PATHS
    global DEBUG_PATHS_LIGHT
    global NUMBER_OF_PACKETS_TO_CONSIDER
    global DEBUG_BRIDGE_CODE_CALLING
    DEBUG_PATHS = false;
    DEBUG_PATHS_LIGHT = false;
    NUMBER_OF_PACKETS_TO_CONSIDER = -1; % Set to -1 to ignore this variable's value
    DEBUG_BRIDGE_CODE_CALLING = false;
    
    % Output controls
    global OUTPUT_AOAS
    global OUTPUT_TOFS
    global OUTPUT_AOA_MUSIC_PEAK_GRAPH
    global OUTPUT_TOF_MUSIC_PEAK_GRAPH
    global OUTPUT_AOA_TOF_MUSIC_PEAK_GRAPH
    global OUTPUT_SELECTIVE_AOA_TOF_MUSIC_PEAK_GRAPH
    global OUTPUT_BINARY_AOA_TOF_MUSIC_PEAK_GRAPH
    global OUTPUT_AOA_VS_TOF_PLOT
    global OUTPUT_SUPPRESSED
    global OUTPUT_PACKET_PROGRESS
    global OUTPUT_FIGURES_SUPPRESSED
    OUTPUT_AOAS = false;
    OUTPUT_TOFS = false;
    OUTPUT_AOA_MUSIC_PEAK_GRAPH = false;
    OUTPUT_TOF_MUSIC_PEAK_GRAPH = false;
    OUTPUT_AOA_TOF_MUSIC_PEAK_GRAPH = false;
    OUTPUT_SELECTIVE_AOA_TOF_MUSIC_PEAK_GRAPH = false;
    OUTPUT_BINARY_AOA_TOF_MUSIC_PEAK_GRAPH = false;
    OUTPUT_AOA_VS_TOF_PLOT = true;
    OUTPUT_SUPPRESSED = false;
    OUTPUT_PACKET_PROGRESS = false;
    OUTPUT_FIGURES_SUPPRESSED = false; % Set to true when running in deployment from command line
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
end