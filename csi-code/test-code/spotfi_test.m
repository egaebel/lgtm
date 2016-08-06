%%
% The MIT License (MIT)
% Copyright (c) 2016 Ethan Gaebel <egaebel@vt.edu>
% 
% Permission is hereby granted, free of charge, to any person obtaining a 
% copy of this software and associated documentation files (the "Software"), 
% to deal in the Software without restriction, including without limitation 
% the rights to use, copy, modify, merge, publish, distribute, sublicense, 
% and/or sell copies of the Software, and to permit persons to whom the 
% Software is furnished to do so, subject to the following conditions:
% 
% The above copyright notice and this permission notice shall be included 
% in all copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
% OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
% FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
% DEALINGS IN THE SOFTWARE.
%

function spotfi_test
    %% DEBUG AND OUTPUT VARIABLES-----------------------------------------------------------------%%
    % Debug Controls
    global DEBUG_BRIDGE_CODE_CALLING
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Get the full path to the currently executing file and change the
    % pwd to the folder this file is contained in...
    [current_directory, ~, ~] = fileparts(mfilename('fullpath'));
    cd(current_directory);
    %pwd
    % Path to data used in this test
    path('..', path);
    path('../../csi-code/test-data/localization-tests--in-room', path);
    path('../../csi-code/test-data/line-of-sight-localization-tests--in-room', path);
    % Paths for the csitool functions provided
    path('../../../linux-80211n-csitool-supplementary/matlab', path);
	path('../../../linux-80211n-csitool-supplementary/matlab/sample_data', path);

    globals_init();
    
    if DEBUG_BRIDGE_CODE_CALLING
        fprintf('The path: %s\n', path)
    end
    if ~DEBUG_BRIDGE_CODE_CALLING
        close all
        clc
    end
    %% First localization tests, without strict line of sight
    %data_files = {
    %    'csi-5ghz-10cm-desk-spacing-printer.dat', ...
    %    'csi-5ghz-10cm-desk-spacing-on-book-shelf-2.dat', ...
    %    'csi-5ghz-10cm-desk-spacing-clothes-hamper.dat', ...
    %    'csi-5ghz-10cm-desk-spacing-on-bed.dat', ...
    %    'csi-5ghz-10cm-desk-spacing-bed-side-power-block.dat', ...
    %    'csi-5ghz-10cm-desk-spacing-bed-side-table.dat'
    %};
    data_files = {
        'los-test-heater.dat' ...
        'los-test-desk-left.dat' ...
        'los-test-desk-right.dat' ...
        'los-test-printer.dat' ...
        'los-test-nearby-long-bookshelf.dat' ...
        'los-test-tall-bookshelf.dat' ...
        'los-test-jennys-table.dat' ...
    };
    run(data_files)
    fprintf('Done Running!\n')
end

%% Runs the SpotFi test over the passed in data files which each contain CSI data for many packets
% data_files -- a cell array of file paths to data files
function run(data_files)
    %% DEBUG AND OUTPUT VARIABLES-----------------------------------------------------------------%%
    % Debug Controls
    global NUMBER_OF_PACKETS_TO_CONSIDER
    
    % Output controls
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    % Set physical layer parameters (frequency, subfrequency spacing, and antenna spacing
    antenna_distance = 0.1;
    % frequency = 5 * 10^9;
    frequency = 5.785 * 10^9;
    % frequency = 5.32 * 10^9;
    sub_freq_delta = (40 * 10^6) / 30;
    
    % Loop over passed in data files
    for data_file_index = 1:length(data_files)
        % Read data file in
        fprintf('\n\nRunning on data file: %s\n', data_files{data_file_index})
        csi_trace = read_bf_file(data_files{data_file_index});
        
        % Extract CSI information for each packet
        fprintf('Have CSI for %d packets\n', length(csi_trace))
        
        % Set the number of packets to consider, by default consider all
        num_packets = length(csi_trace);
        if NUMBER_OF_PACKETS_TO_CONSIDER ~= -1
            num_packets = NUMBER_OF_PACKETS_TO_CONSIDER;
        end
        fprintf('Considering CSI for %d packets\n', num_packets)
        sampled_csi_trace = csi_sampling(csi_trace, num_packets);
        output_top_aoas = spotfi(sampled_csi_trace, frequency, sub_freq_delta, antenna_distance, ...
                data_files{data_file_index})
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%