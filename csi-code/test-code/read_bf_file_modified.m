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
% Original code provided by Daniel Halperin with the original copywrite
% information below

%READ_BF_FILE_MODIFIED Reads in a file of beamforming feedback logs.
%   This version uses the *C* version of read_bfee, compiled with
%   MATLAB's MEX utility.
%
% (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
%
function ret = read_bf_file_modified(filename)

    % Add path information for read_bfee (this is a testing file so don't
    % worry yourself about whether this is the proper place to do this)
    path('../../../linux-80211n-csitool-supplementary/matlab', path);
    
    fprintf('read_bf_file_modified called: checking nargchk....\n')
    %% Input check
    error(nargchk(1,1,nargin));
    
    %% Open file
    f = fopen(filename, 'rb');
    if (f < 0)
        error('Couldn''t open file %s', filename);
        return;
    end

    status = fseek(f, 0, 'eof');
    if status ~= 0
        [msg, errno] = ferror(f);
        fclose(f);
        error('Error %d seeking: %s', errno, msg);
        return;
    end
    len = ftell(f);

    status = fseek(f, 0, 'bof');
    if status ~= 0
        [msg, errno] = ferror(f);
        fclose(f);
        error('Error %d seeking: %s', errno, msg);
        return;
    end

    %% Initialize variables
    ret = cell(ceil(len/95),1);     % Holds the return values - 1x1 CSI is 95 bytes big, so this should be upper bound
    cur = 0;                        % Current offset into file
    count = 0;                      % Number of records output
    payload_count = 0;              % Number of packet payloads outpu
    broken_perm = 0;                % Flag marking whether we've encountered a broken CSI yet
    triangle = [1 3 6];             % What perm should sum to for 1,2,3 antennas

    payload_file = fopen('payload_file', 'wb');
    if (payload_file < 0)
        error('Couldn''t open file %s', 'payload_file');
        return;
    end
    num_bytes_written = 0;
    
    %% Process all entries in file
    % Need 3 bytes -- 2 byte size field and 1 byte code
    while cur < (len - 3)
        % Read size and code
        field_len = fread(f, 1, 'uint16', 0, 'ieee-be');
        code = fread(f,1);
        cur = cur+3;
        fprintf('field_len - 1: %d\n', (field_len - 1))
        fprintf('code: %d\n', code)
        % If unhandled code, skip (seek over) the record and continue
        if (code == 187) % get beamforming or phy data
            fprintf('yay, phy data!\n')
            bytes = fread(f, field_len-1, 'uint8=>uint8');
            cur = cur + field_len - 1;
            if (length(bytes) ~= field_len-1)
                fclose(f);
                return;
            end
        elseif (code == 193) %hex2dec('c1')) Packet MPDU -- output the payload
            payload_count = payload_count + 1;
            bytes = fread(f, field_len-1, 'uint8=>uint8');
            % data is in bytes: 25-length(bytes) - 4
            %fprintf('Full bytes: \n')
            %dec2hex(bytes)
            %fprintf('Sliced version: \n')
            %dec2hex(bytes(25:length(bytes) - 4))
            fwrite(payload_file, bytes(25:length(bytes) - 4), 'uint8');
            %fprintf('length(bytes): %d\n', length(bytes))
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

        if (code == 187) %hex2dec('bb')) Beamforming matrix -- output a record
            count = count + 1;
            ret{count} = read_bfee(bytes);
            fprintf('Result from read_bfee\n')
            ret{count}
            perm = ret{count}.perm;
            Nrx = ret{count}.Nrx;
            if Nrx == 1 % No permuting needed for only 1 antenna
                continue;
            end
            if sum(perm) ~= triangle(Nrx) % matrix does not contain default values
                if broken_perm == 0
                    broken_perm = 1;
                    fprintf('WARN ONCE: Found CSI (%s) with Nrx=%d and invalid perm=[%s]\n', filename, Nrx, int2str(perm));
                end
            else
                ret{count}.csi(:,perm(1:Nrx),:) = ret{count}.csi(:,1:Nrx,:);
            end
        end
    end

    ret = ret(1:count);

    fprintf('payload count: %d\n', payload_count)
    
    %% Close files
    fclose(f);
    fclose(payload_file);
    fprintf('Exiting read_bf_file normally...\n')
end
