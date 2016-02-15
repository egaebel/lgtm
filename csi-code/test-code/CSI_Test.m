function CSI_Test
    csi_trace = read_bf_file('csi-5ghz-10cm-desk-spacing-on-book-case.dat');
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

