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
        input_filename = 'lgtm-monitor.dat';
        output_filename = 'lgtm-monitor-check';
    else
        narginchk(2, 2);
    end
    
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
        error('Couldn''t open file %s', 'payload_file');
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