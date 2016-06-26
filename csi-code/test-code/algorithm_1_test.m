

%% Tests Algorithm 1 using the passed in data_file
% data_file -- the data file to use for testing Algorithm 1
function algorithm_1_test

    data_file = 'csi-5ghz-10cm-desk-spacing-printer.dat';

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
