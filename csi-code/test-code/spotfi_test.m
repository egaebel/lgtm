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
    globals_init();
    % Get the full path to the currently executing file and change the
    % pwd to the folder this file is contained in...
    [current_directory, ~, ~] = fileparts(mfilename('fullpath'));
    cd(current_directory);
    pwd
    % Path to data used in this test
    path('..', path);
    path('../../csi-code/test-data/localization-tests--in-room', path);
    path('../../csi-code/test-data/line-of-sight-localization-tests--in-room', path);
    % Paths for the csitool functions provided
    path('../../../linux-80211n-csitool-supplementary/matlab', path);
	path('../../../linux-80211n-csitool-supplementary/matlab/sample_data', path);

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
    OUTPUT_AOA_VS_TOF_PLOT = true;
    OUTPUT_SUPPRESSED = false;
    OUTPUT_PACKET_PROGRESS = false;
    OUTPUT_FIGURES_SUPPRESSED = false; % Set to true when running in deployment from command line
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

%% Time of Flight (ToF) Sanitization Algorithm, find a linear fit for the unwrapped CSI phase
% csi_matrix -- the CSI matrix whose phase is to be adjusted
% delta_f    -- the difference in frequency between subcarriers
% Return:
% csi_matrix -- the same CSI matrix with modified phase
function [csi_matrix, phase_matrix] = spotfi_algorithm_1(csi_matrix, delta_f, packet_one_phase_matrix)
    %% Time of Flight (ToF) Sanitization Algorithm
    %  Obtain a linear fit for the phase
    %  Using the expression:
    %      argmin{\rho} \sum_{m,n = 1}^{M, N} (\phi_{i}(m, n) 
    %          + 2 *\pi * f_{\delta} * (n - 1) * \rho + \beta)^2
    %
    %  Arguments:
    %  M is the number of antennas
    %  N is the number of subcarriers
    %  \phi_{i}(m, n) is the phase for the nth subcarrier, 
    %      on the mth antenna, for the ith packet
    %  f_{\delta} is the frequency difference between the adjacent
    %      subcarriers
    %  \rho and \beta are the linear fit variables
    %
    % Unwrap phase from CSI matrix
    R = abs(csi_matrix);
    phase_matrix = unwrap(angle(csi_matrix), pi, 2);
    
    % Parse input args
    if nargin < 3
        packet_one_phase_matrix = phase_matrix;
    end

    % STO is the same across subcarriers....
    % Data points are:
    % subcarrier_index -> unwrapped phase on antenna_1
    % subcarrier_index -> unwrapped phase on antenna_2
    % subcarrier_index -> unwrapped phase on antenna_3
    fit_X(1:30, 1) = 1:1:30;
    fit_X(31:60, 1) = 1:1:30;
    fit_X(61:90, 1) = 1:1:30;
    fit_Y = zeros(90, 1);
    for i = 1:size(phase_matrix, 1)
        for j = 1:size(phase_matrix, 2)
            fit_Y((i - 1) * 30 + j) = packet_one_phase_matrix(i, j);
        end
    end

    % Linear fit is common across all antennas
    result = polyfit(fit_X, fit_Y, 1);
    tau = result(1);
        
    for m = 1:size(phase_matrix, 1)
        for n = 1:size(phase_matrix, 2)
            % Subtract the phase added from sampling time offset (STO)
            phase_matrix(m, n) = packet_one_phase_matrix(m, n) + (2 * pi * delta_f * (n - 1) * tau);
        end
    end
    
    % Reconstruct the CSI matrix with the adjusted phase
    csi_matrix = R .* exp(1i * phase_matrix);
end

function [csi_matrix] = pin_loc_sanitization_algorithm(csi_matrix)
    % Unwrap phase from CSI matrix
    R = abs(csi_matrix);
    phase_matrix = unwrap(angle(csi_matrix), pi, 2);
    
    for i = 1:size(phase_matrix, 1)
        a = (phase_matrix(i, size(phase_matrix, 2)) - phase_matrix(i, 1)) / (2 * pi * 30);
        b = 1 / size(phase_matrix, 2) * sum(phase_matrix(i, :));
        for j = 1:size(phase_matrix, 2)
            phase_matrix(i, j) = phase_matrix(i, j) - a * j - b;
        end
    end
    
    % Reconstruct the CSI matrix with the adjusted phase
    csi_matrix = R .* exp(1i * phase_matrix);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Tests Algorithm 1 using the passed in data_file
% data_file -- the data file to use for testing Algorithm 1
function test_algorithm_1(data_file)
    fprintf('Testing Algorithm 1\n')
    % Read data file in
    fprintf('Running on data file: %s\n', data_file)
    csi_trace = read_bf_file(data_file);
    % Extract CSI information for each packet
    fprintf('Have CSI for %d packets\n', length(csi_trace))

    % Get CSI for the first packet
    csi_entry_1 = csi_trace{1};
    csi_entry_2 = csi_trace{2};
    csi_1 = get_scaled_csi(csi_entry_1);
    csi_2 = get_scaled_csi(csi_entry_2);
    % Only consider CSI from transmission on 1 antenna
    csi_1 = csi_1(1, :, :);
    csi_2 = csi_2(1, :, :);
    % Remove the single element dimension
    csi_1 = squeeze(csi_1);
    csi_2 = squeeze(csi_2);
    % Sanitize ToFs with Algorithm 1
    % delta_f for HT20
    %delta_f = 20 * 10^6 / 30;
    % delta_f for HT40
    delta_f = 40 * 10^6 / 30;
    fprintf('Running SpotFi Algorithm 1\n')
    [modified_csi_1, packet_one_phase_matrix] = spotfi_algorithm_1(csi_1, delta_f);
    [modified_csi_2, packet_two_phase_matrix] = spotfi_algorithm_1(csi_2, delta_f, unwrap(angle(csi_1), pi, 2));
    %modified_csi_1 = pin_loc_sanitization_algorithm(csi_1);
    %modified_csi_2 = pin_loc_sanitization_algorithm(csi_1);
    
    % Unmodified figures
    figure('Name', 'Unmodified CSI Phase', 'NumberTitle', 'off')
    hold on
    csi_1_phase = unwrap(angle(csi_1), pi, 2);
    plot(1:1:30, csi_1_phase(1, :), '-k')
    plot(1:1:30, csi_1_phase(2, :), '-r')
    plot(1:1:30, csi_1_phase(3, :), '-g')
    
    csi_2_phase = unwrap(angle(csi_2), pi, 2);
    plot(1:1:30, csi_2_phase(1, :), '--k')
    plot(1:1:30, csi_2_phase(2, :), '--r')
    plot(1:1:30, csi_2_phase(3, :), '--g')
    xlabel('Subcarrier Index')
    ylabel('Unwrapped CSI Phase')
    title('Unmodified CSI Phase')
    legend('Packet 1, Antenna 1', 'Packet 1, Antenna 2', 'Packet 1, Antenna 3', ...
            'Packet 2, Antenna 1', 'Packet 2, Antenna 2', 'Packet 2, Antenna 3')
    grid on
    hold off
    
    % Modified CSI Phase
    figure('Name', 'Modified CSI Phase', 'NumberTitle', 'off')
    hold on
    
    modified_csi_1_phase = unwrap(angle(modified_csi_1), pi, 2);
    plot(1:1:30, modified_csi_1_phase(1, :), '-k')
    plot(1:1:30, modified_csi_1_phase(2, :), '-r')
    plot(1:1:30, modified_csi_1_phase(3, :), '-g')
    
    modified_csi_2_phase = unwrap(angle(modified_csi_2), pi, 2);
    plot(1:1:30, modified_csi_2_phase(1, :), '^--k', 'LineWidth', 3)
    plot(1:1:30, modified_csi_2_phase(2, :), '^--r', 'LineWidth', 3)
    plot(1:1:30, modified_csi_2_phase(3, :), '^--g', 'LineWidth', 3)
    
    xlabel('Subcarrier Index')
    ylabel('Modified Unwrapped CSI Phase')
    title('Modified CSI Phase')
    legend('Antenna 1, Packet 1', 'Antenna 2, Packet 1', 'Antenna 3, Packet 1', ...
            'Antenna 1, Packet 2', 'Antenna 2, Packet 2', 'Antenna 3, Packet 2')
    grid on
    hold off
    return
end
