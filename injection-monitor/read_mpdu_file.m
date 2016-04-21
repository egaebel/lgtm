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

%READ_MPDU_FILE Reads in file of Media Access Control Protocol Data Unit 
%       (MPDU) logs and outputs the parsed data bytes to a file.
%   This version uses the *C* version of read_bfee, compiled with
%   MATLAB's MEX utility.
%
% (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
%
function read_mpdu_file(input_filename, output_filename)
    fprintf('read_mpdu_file called: checking nargchk....\n')
    %% Input check
    if nargin == 0
        input_filename = '.lgtm-monitor.dat';
        output_filename = '.lgtm-received-facial-recognition-params';
        fprintf('No arguments given to read_mpdu_file....reverting to defaults: %s and %s\n', ...
                input_filename, output_filename)
    else
        narginchk(2, 2);
        fprintf('Received input and output arugments: %s and %s\n', ...
                input_filename, output_filename)
    end
    
    % Get the full path to the currently executing file and change the
    % pwd to the folder this file is contained in...
    [current_directory, ~, ~] = fileparts(mfilename('fullpath'));
    cd(current_directory);
    
    %% Open file
    f = fopen(input_filename, 'rb');
    if (f < 0)
        error('Couldn''t open file %s', input_filename);
    end

    status = fseek(f, 0, 'eof');
    if status ~= 0
        [msg, errno] = ferror(f);
        fclose(f);
        error('Error %d seeking: %s', errno, msg);
    end
    len = ftell(f);

    status = fseek(f, 0, 'bof');
    if status ~= 0
        [msg, errno] = ferror(f);
        fclose(f);
        error('Error %d seeking: %s', errno, msg);
    end

    %% Initialize variables
    cur = 0;                        % Current offset into file
    payload_count = 0;              % Number of packet payloads outpu

    payload_file = fopen(output_filename, 'wb');
    if (payload_file < 0)
        error('Couldn''t open file %s', output_filename);
    end
    num_bytes_written = 0;
    
    fprintf('Processing file entries....\n')
    %% Process all entries in file
    % Need 3 bytes -- 2 byte size field and 1 byte code
    while cur < (len - 3)
        % Read size and code
        field_len = fread(f, 1, 'uint16', 0, 'ieee-be');
        code = fread(f, 1);
        cur = cur + 3;
        % If unhandled code, skip (seek over) the record and continue
        if (code == 193) %hex2dec('c1')) Packet MPDU -- output the payload
            payload_count = payload_count + 1;
            bytes = fread(f, field_len-1, 'uint8=>uint8');
            % data is in bytes: 25-length(bytes) - 4
            fwrite(payload_file, bytes(25:length(bytes) - 4), 'uint8');
            num_bytes_written = num_bytes_written + length(bytes);
            continue
        elseif isempty(field_len)
            fprintf('Field length is empty...break\n')
            break;
        else % skip all other info
            fseek(f, field_len - 1, 'cof');
            cur = cur + field_len - 1;
            continue;
        end
    end

    fprintf('Closing files....\n')
    %% Close files
    fclose(f);
    fclose(payload_file);
end