function CSI_Test
    %data_files = {
        %'los-test-desk-left.dat' ...
        %'los-test-desk-right.dat' ...
        %'los-test-heater.dat' ...
        %'los-test-jennys-table.dat' ...
        %'los-test-nearby-long-bookshelf.dat' ...
        %'los-test-printer.dat' ...
        %'los-test-tall-bookshelf.dat' ...
    %};

    % Get the full path to the currently executing file and change the
    % pwd to the folder this file is contained in...
    [current_directory, ~, ~] = fileparts(mfilename('fullpath'));
    cd(current_directory);
    % Paths for the csitool functions provided
    path('../../../linux-80211n-csitool-supplementary/matlab/', path);
	path('../../../linux-80211n-csitool-supplementary/matlab/sample_data/', path);
    path('../test-data', path);
    path('../test-data/line-of-sight-localization--in-room', path);
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %csi_trace = read_bf_file_modified('../test-data/monitor-log-deadbeef-file-long.dat');
    csi_trace = read_bf_file('los-test-tall-bookshelf.dat');
    csi_entry = csi_trace{1}
    fprintf('Number of Packet Traces %d\n', length(csi_trace))
    csi = get_scaled_csi(csi_entry);
    csi = csi(1, :, :);
    plot(db(abs(squeeze(csi).')))
    legend('RX Antenna A', 'RX Antenna B', 'RX Antenna C','Location', 'SouthEast' );
    xlabel('Subcarrier Index');
    ylabel('SNR [dB]');
    
    % The structure of this matrix is:
    % 1x Antenna1 Antenna2 (Antenna3) x rows of subcarriers
    % csi_entry.csi
    
    % ndims(csi_entry.csi)
    % size(csi_entry.csi(1))
    % size(csi_entry.csi(2))
    % size(csi_entry.csi(3))
    % csi_entry.csi(:, :, :)
    % fprintf('-----------')
    % csi_entry.csi(:, :, 1)
    % fprintf('-----------')
    % csi_entry.csi(:, 1, 1)
end