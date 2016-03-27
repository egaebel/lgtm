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

%% Tests the csi_sampling function
function csi_sampling_test
    clc
    [current_directory, ~, ~] = fileparts(mfilename('fullpath'));
    cd(current_directory);
    path('../../../linux-80211n-csitool-supplementary/matlab', path);
    path('..', path);
    
    % Set physical layer parameters (frequency, subfrequency spacing, and antenna spacing
    antenna_distance = 0.1;
    % frequency = 5 * 10^9;
    frequency = 5.785 * 10^9;
    sub_freq_delta = (40 * 10^6) / 30;
    
    data_files = {
        'los-test-heater.dat' ...
        'los-test-desk-left.dat' ...
        'los-test-desk-right.dat' ...
        'los-test-printer.dat' ...
        'los-test-nearby-long-bookshelf.dat' ...
        'los-test-tall-bookshelf.dat' ...
        'los-test-jennys-table.dat' ...
    };
    
    % PARAMETERS------------------------------------------------------------------------------------
    NUM_SAMPLED_PACKETS = 150;
    
    for ii = 1:length(data_files)
        fprintf('Running on data file: %s\n', data_files{ii})
        data_file = sprintf('../test-data/line-of-sight-localization-tests--in-room/%s', data_files{ii});
        csi_trace = read_bf_file(data_file);
        
        fprintf('csi_trace size: %d\n', length(csi_trace))
        fprintf('Sampling size: %d\n', NUM_SAMPLED_PACKETS)
        sampled_csi_trace = csi_sampling(csi_trace, NUM_SAMPLED_PACKETS);
        % Print out all the sampled items
        %{
        for ii = 1:length(sampled_csi_trace)
            sampled_csi_trace{ii}
        end
        %}
        fprintf('\nAoAs with full length CSI\n')
        spotfi(csi_trace, frequency, sub_freq_delta, antenna_distance)

        fprintf('AoAs with sampled CSI\n')
        spotfi(sampled_csi_trace)
    end
    
    fprintf('\n\nDone!\n\n')
end