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

%% Takes a cell array of CSI information and samples n packets uniformly from the information
% csi_data        -- The csi data, in a cell array, to sample from
% n               -- The number of packets to sample from
% alt_begin_index -- Optional: The index to begin sampling from, if omitted this defaults to 1
% alt_end_index   -- Optional: The index to end sampling from, if omitted this 
%                       defaults to length(csi_data)
function sampled_csi = csi_sampling(csi_data, n, alt_begin_index, alt_end_index)
    % Variable number of arguments handling
    if nargin < 3
        begin_index = 1;
        end_index = length(csi_data);
    elseif nargin < 4
        begin_index = alt_begin_index;
        end_index = length(csi_data);
    elseif nargin == 4
        begin_index = alt_begin_index;
        end_index = alt_end_index;
    end
    
    % Sampling
    sampling_interval = floor((end_index - begin_index + 1) / n);
    sampled_csi = cell(n, 1);
    jj = 1;
    for ii = begin_index:sampling_interval:end_index
        % Get CSI for current packet
        sampled_csi{jj} = csi_data{ii};
        jj = jj + 1;
    end
end