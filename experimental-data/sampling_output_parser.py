#!/usr/bin/python3

import sys
import os
import math

DISTANCE_KEY = 'DISTANCE'
ANGLE_KEY = 'ANGLE'
LAPTOP_ID_KEY = 'LAPTOP_ID'
TEST_NUMBER_KEY = 'TEST_NUMBER'
SAMPLING_PARAMETERS_DICT_KEY = 'SAMPLING_PARAMETERS'
# Keys to the sampling parameters dict
FILE_NAME_KEY = 'file name'
NUM_SAMPLES_KEY = 'number of samples'
SAMPLING_BEGIN_INDEX_KEY = 'sampling beginning index'
SAMPLING_END_INDEX_KEY = 'sampling ending index'

TOP_AOA_INDEX_KEY = 'TOP_AOA_INDEX'
TOP_AOA_KEY = 'TOP_AOA'
TOP_AOAS_LIST_KEY = 'TOP_AOAS_LIST'

AOA_MATCHES_KEY = 'AOA_MATCHES'

# Parse the file output from sampling
def parse_spotfi_sampling_output(folder_name):
    # Leading text before data of interest
    DATA_FILE_HEADER = 'Running on data file:'
    NUM_PACKETS_HEADER = 'number of packets ='
    BEGIN_INDEX_HEADER = 'begin index ='
    END_INDEX_HEADER = 'end_index ='
    TOP_AOAS_HEADER = 'output_top_aoas ='

    data_files_data = dict()
    sampling_parameters_data = dict()
    top_aoas = list()

    cur_data_file = None
    cur_num_packets = None
    cur_begin_index = None
    cur_end_index = None
    cur_top_aoa = None
    cur_top_aoa_status = False

    # Iterate over all files in the passed directory
    # Don't stick anything you don't want touched in there....
    for sample_file_name in os.listdir(folder_name):
        sample_file_name = os.path.join(folder_name, sample_file_name)
        if not os.path.isdir(sample_file_name):
            # Open each file
            with open(sample_file_name, 'r') as monitor_output_file:
                # Iterate over each file
                for line in monitor_output_file:
                    line = line.strip()
                    # Skip blank lines
                    if not line:
                        continue
                    if line.find(DATA_FILE_HEADER) != -1:
                        # Check if we have top_aoas from the prior iterations
                        # This needs to be in two places since the data following
                        # the AoAs may be a new data file, OR new sampling data
                        if len(top_aoas) > 0:
                            sampling_key = cur_data_file + ' ' + cur_num_packets + ' '\
                                    + cur_begin_index + ' ' + cur_end_index
                            sampling_parameters_data[sampling_key] = top_aoas
                            top_aoas = list()
                        # Check if we have sampling_parameters data from the prior iterations
                        if len(sampling_parameters_data) > 0:
                            data_files_data[cur_data_file] = sampling_parameters_data
                            sampling_parameters_data = dict()
                        # Do regular parsing for DATA_FILE_HEADER
                        cur_data_file = line[len(DATA_FILE_HEADER):].strip()
                    elif line.find(NUM_PACKETS_HEADER) != -1:
                        # Reset top_aoa_status
                        cur_top_aoa_status = False
                        # Check if we have top_aoas from the prior iterations
                        # This needs to be in two places since the data following
                        # the AoAs may be a new data file, OR new sampling data
                        if len(top_aoas) > 0:
                            sampling_key = cur_data_file + ' ' + cur_num_packets + ' '\
                                    + cur_begin_index + ' ' + cur_end_index
                            sampling_parameters_data[sampling_key] = top_aoas
                            top_aoas = list()
                        # Do regular parsing for NUM_PACKETS_HEADER
                        cur_num_packets = line[len(NUM_PACKETS_HEADER):].strip()
                    elif line.find(BEGIN_INDEX_HEADER) != -1:
                        cur_begin_index = line[len(BEGIN_INDEX_HEADER):].strip()
                    elif line.find(END_INDEX_HEADER) != -1:
                        cur_end_index = line[len(END_INDEX_HEADER):].strip()
                    elif line.find(TOP_AOAS_HEADER) != -1:
                        cur_top_aoa_status = True
                    elif cur_top_aoa_status:
                        try:
                            cur_top_aoa = float(line)
                            top_aoas.append(cur_top_aoa)
                        except:
                            cur_top_aoa_status = False

    # Check if we have top_aoas from the final iteration
    if len(top_aoas) > 0:
        sampling_key = cur_data_file + ' ' + cur_num_packets + ' '\
                + cur_begin_index + ' ' + cur_end_index
        sampling_parameters_data[sampling_key] = top_aoas
        top_aoas = list()
    # Check if we have sampling_parameters data from the final iteration
    if len(sampling_parameters_data) > 0:
        data_files_data[cur_data_file] = sampling_parameters_data

    return data_files_data

# Extract data from file names, that is: distance, degrees, laptop id, and test number
# Return the data in a dictionary keyed with: 
#     DISTANCE_KEY, ANGLE_KEY, LAPTOP_ID_KEY, TEST_NUMBER_KEY
# Sample file format: lgtm-monitor.dat--2m-neg-20-degrees--laptop-2--test-9
def parse_file_name(file_name):
    FILE_HEADER = 'lgtm-monitor.dat--'
    NEGATIVE_INDICATOR = '-neg-'
    DASH = '-'
    UNIT_INDICATOR = 'm'
    DEGREE_INDICATOR = '-degrees--'
    LAPTOP_INDICATOR = 'laptop-'
    TEST_INDICATOR = '--test-'

    distance = None
    angle = None
    laptop_id = None
    test_number = None
    file_data = dict()

    file_name = file_name[len(FILE_HEADER):]
    
    # Distance parse
    unit_index = file_name.find(UNIT_INDICATOR)
    distance = int(file_name[:unit_index])
    file_name = file_name[(unit_index + len(UNIT_INDICATOR)):]

    # Degrees parse
    negative_indicator_index = file_name.find(NEGATIVE_INDICATOR)
    if negative_indicator_index != -1:
        file_name = file_name[len(NEGATIVE_INDICATOR):]
        angle = '-'
    else:
        file_name = file_name[len(DASH):]
        angle = ''
    degree_indicator_index = file_name.find(DEGREE_INDICATOR)
    angle += file_name[:degree_indicator_index]
    file_name = file_name[degree_indicator_index + len(DEGREE_INDICATOR):]
    # SPECIAL CASE TO FIX MY TYPO IN CREATING FILES
    if angle == '90':
        angle = '0'

    # Laptop ID parse
    file_name = file_name[len(LAPTOP_INDICATOR):]
    test_indicator_index = file_name.find(TEST_INDICATOR)
    laptop_id = file_name[:test_indicator_index]
    file_name = file_name[test_indicator_index + len(TEST_INDICATOR):]

    # Test number parse
    test_number = file_name

    file_data[DISTANCE_KEY] = distance
    file_data[ANGLE_KEY] = angle
    file_data[LAPTOP_ID_KEY] = laptop_id
    file_data[TEST_NUMBER_KEY] = test_number

    return file_data

# Takes a string in the form: 
#     <file_name> <number of samples> <sampling begin index> <sampling end index>
def parse_sampling_parameters_key(sampling_parameters_key):
    
    parameters_list = sampling_parameters_key.split(' ')
    file_name = parameters_list[0]
    number_of_samples = parameters_list[1]
    sampling_begin_index = parameters_list[2]
    sampling_end_index = parameters_list[3]

    return_dict = {FILE_NAME_KEY: file_name, 
            NUM_SAMPLES_KEY: number_of_samples,
            SAMPLING_BEGIN_INDEX_KEY: sampling_begin_index,
            SAMPLING_END_INDEX_KEY: sampling_end_index}

    return return_dict


# Take the distance between nodes, true angle, and a list of angles
# Output a dictionary with the index and angle of any angles that match
# the true angle (within 40 cm tolerance (on either side))
def compare_true_and_measured_angles(distance, true_angle, top_aoas):
    # If the distance between the angles is greater than 40 cm
    DISTANCE_MARGIN = 0.4
    # DISTANCE_MARGIN = 1.0
    matching_indices = list()
    for i in range(len(top_aoas)):

        true_location = distance * math.sin(math.radians(float(true_angle)))
        measured_location = distance * math.sin(math.radians(float(top_aoas[i])))

        """
        print("y-distance: %g" % distance)
        print("True Angle: %g, True distance: %g" % (float(true_angle), true_location))
        print("Upper bound distance: %g" % (DISTANCE_MARGIN + true_location))
        print("top_aoa[%d]: %g, distance: %g" % (i, float(top_aoas[i]), measured_location))
        print("Lower bound distance: %g" % (-DISTANCE_MARGIN + true_location))
        """
        # See if the current element in top_aoas is within the valid range to be considered "right"
        if (DISTANCE_MARGIN + true_location > measured_location)\
                and measured_location > (-DISTANCE_MARGIN + true_location):
            top_aoa_dict = dict()
            top_aoa_dict[TOP_AOA_INDEX_KEY] = i
            top_aoa_dict[TOP_AOA_KEY] = top_aoas[i]
            matching_indices.append(top_aoa_dict)

    return matching_indices

# Take a list of dictionaries where each has the keys: ANGLE_KEY, DISTANCE_KEY, and AOA_MATCHES_KEY
# Runs over all of them and computes the error rate by looking at how many
#       values under AOA_MATCHES are empty lists
# If the distance parameters is given only the entries under DISTANCE_KEY which
#       match the given distance will be considered. 
#       The distance argument to this function should be an integer.
# If the angle parameter is given only the entries under ANGLE_KEY which match
#       the given angle will be considered. 
#       The angle argument to this function should be an integer.
def error_rate(data_list, distance=None, angle=None, laptop_id=None, num_samples=None, suppress_output=False):
    total_entries_considered = 0
    complete_top_3_error_count = 0
    complete_top_5_error_count = 0
    top_correct_count = [0 for x in range(5)]
    for entry in data_list:
        # Skip this entry if we care about the angle and it doesn't match up
        if angle is not None and int(entry[ANGLE_KEY]) != angle:
            continue
        # Skip this entry if we care about distance and it doesn't match up
        if distance is not None and int(entry[DISTANCE_KEY]) != distance:
            continue
        # Skip this entry if we care about laptop id and it doesn't match up
        if laptop_id is not None and int(entry[LAPTOP_ID_KEY]) != laptop_id:
           continue
        # Skip this entry if we care about the number of samples and it doesn't match up
        if num_samples is not None\
                and int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY]) != num_samples:
            continue
        # Skip this entry if the number of samples is not equivalent to the full spread of packets
        # This runs counter to the case above
        if num_samples is None:
            cur_num_samples = int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY])
            cur_end_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_END_INDEX_KEY])
            cur_begin_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_BEGIN_INDEX_KEY])
            # If the current number of samples is not equal to the total count of packets, skip
            #   I need to subtract one because these indices are from MATLAB, so they start at 1
            if cur_num_samples != cur_end_index - (cur_begin_index - 1):
                continue

        if len(entry[AOA_MATCHES_KEY]) == 0:
            complete_top_3_error_count += 1
            complete_top_5_error_count += 1
        else:
            # Add to top_3_error_count if there are no top angle choices in the top 3 indices
            # I'm comparing to indices, so I need to use the >= for the 3
            if min(entry[AOA_MATCHES_KEY], key=lambda x: x[TOP_AOA_INDEX_KEY])[TOP_AOA_INDEX_KEY] >= 3:
                complete_top_3_error_count += 1

            # Count up the number of correct matches in each position and 
            #	 include the positions before (top 1, top 2, top 3, etc)
            for aoa_match in entry[AOA_MATCHES_KEY]:
                entry_number = aoa_match[TOP_AOA_INDEX_KEY]
                for i in range(entry_number, len(top_correct_count)):
                    top_correct_count[i] += 1
                break;

        total_entries_considered += 1

    # Convert counts for each position to percent correct and report
    top_percent_correct = []
    for i in range(len(top_correct_count)):
        num_in_top_i = top_correct_count[i]
        percent_correct = num_in_top_i / total_entries_considered * 100
        # Add one to convert from zero-indexing
        if not suppress_output:
            print("Percent correct in top %d: %g" % ((i + 1), percent_correct))
        top_percent_correct.append(percent_correct)

    if not suppress_output:
        # Report top level statistics
        print("Total number of samples: %d" % total_entries_considered)
        print("Total number of errors for top 3 (No elements in top 3): %d" % complete_top_3_error_count)
        print("Total number of errors for top 5 (No elements in top 5): %d" % complete_top_5_error_count)

    return top_percent_correct

# Compute the mean error in data_list
# If distance and/or angle are passed then only entries matching distance and/or angle
#       will be considered
# Returns the mean error
def mean_error(data_list, distance=None, angle=None, laptop_id=None, num_samples=None, suppress_output=False):
    total_entries_considered = 0
    error_sum = 0
    for entry in data_list:
        # Skip this entry if we care about the angle and it doesn't match up
        if angle is not None and int(entry[ANGLE_KEY]) != angle:
            continue
        # Skip this entry if we care about distance and it doesn't match up
        if distance is not None and int(entry[DISTANCE_KEY]) != distance:
            continue
        # Skip this entry if we care about laptop id and it doesn't match up
        if laptop_id is not None and int(entry[LAPTOP_ID_KEY]) != laptop_id:
            continue
        # Skip this entry if we care about the number of samples and it doesn't match up
        if num_samples is not None\
                and int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY]) != num_samples:
            continue
        # Skip this entry if the number of samples is not equivalent to the full spread of packets
        # This runs counter to the case above
        if num_samples is None:
            cur_num_samples = int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY])
            cur_end_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_END_INDEX_KEY])
            cur_begin_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_BEGIN_INDEX_KEY])
            # If the current number of samples is not equal to the total count of packets, skip
            #   I need to subtract one because these indices are from MATLAB, so they start at 1
            if cur_num_samples != cur_end_index - (cur_begin_index - 1):
                continue

        top_aoa = entry[TOP_AOAS_LIST_KEY][0]
        true_aoa = float(entry[ANGLE_KEY])
        
        true_distance = entry[DISTANCE_KEY]
        top_location = true_distance * math.sin(top_aoa)
        true_location = true_distance * math.sin(true_aoa)
        error = math.fabs(true_location - top_location)

        error_sum += error
        total_entries_considered += 1

    if not suppress_output:
        print("error_sum: " + str(error_sum))
        print("total entries considered: " + str(total_entries_considered))
    mean_error = error_sum / total_entries_considered
    if not suppress_output:
        print("Mean Error: %g" % mean_error)
    return mean_error

# Computes the median error in data_list
# If distance and/or angle are passed then only entries matching distance and/or angle
#       will be considered
# Returns the median error
def median_error(data_list, distance=None, angle=None, laptop_id=None, num_samples=None, suppress_output=False):
    errors = list()
    for entry in data_list:
        # Skip this entry if we care about the angle and it doesn't match up
        if angle is not None and int(entry[ANGLE_KEY]) != angle:
            continue
        # Skip this entry if we care about distance and it doesn't match up
        if distance is not None and int(entry[DISTANCE_KEY]) != distance:
            continue
        # Skip this entry if we care about laptop id and it doesn't match up
        if laptop_id is not None and int(entry[LAPTOP_ID_KEY]) != laptop_id:
            continue
        # Skip this entry if we care about the number of samples and it doesn't match up
        if num_samples is not None\
                and int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY]) != num_samples:
            continue
        # Skip this entry if the number of samples is not equivalent to the full spread of packets
        # This runs counter to the case above
        if num_samples is None:
            cur_num_samples = int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY])
            cur_end_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_END_INDEX_KEY])
            cur_begin_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_BEGIN_INDEX_KEY])
            # If the current number of samples is not equal to the total count of packets, skip
            #   I need to subtract one because these indices are from MATLAB, so they start at 1
            if cur_num_samples != cur_end_index - (cur_begin_index - 1):
                continue

        top_aoa = entry[TOP_AOAS_LIST_KEY][0]
        true_aoa = float(entry[ANGLE_KEY])
        
        true_distance = entry[DISTANCE_KEY]
        top_location = true_distance * math.sin(top_aoa)
        true_location = true_distance * math.sin(true_aoa)
        error = math.fabs(true_location - top_location)
        errors.append(error)

    errors.sort()
    median_error = errors[math.floor(len(errors) / 2)]
    if not suppress_output:
        print("Median Error: %g" % median_error)
    return median_error

# Finds the min and max errors in data_list
# If distance and/or angle are passed then only entries matching distance and/or angle
#       will be considered
# Returns the min and max errors, in that order
def min_max_error(data_list, distance=None, angle=None, laptop_id=None, num_samples=None, suppress_output=False):
    errors = list()
    for entry in data_list:
        # Skip this entry if we care about the angle and it doesn't match up
        if angle is not None and int(entry[ANGLE_KEY]) != angle:
            continue
        # Skip this entry if we care about distance and it doesn't match up
        if distance is not None and int(entry[DISTANCE_KEY]) != distance:
            continue
        # Skip this entry if we care about laptop id and it doesn't match up
        if laptop_id is not None and int(entry[LAPTOP_ID_KEY]) != laptop_id:
            continue
        # Skip this entry if we care about the number of samples and it doesn't match up
        if num_samples is not None\
                and int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY]) != num_samples:
            continue
        # Skip this entry if the number of samples is not equivalent to the full spread of packets
        # This runs counter to the case above
        if num_samples is None:
            cur_num_samples = int(entry[SAMPLING_PARAMETERS_DICT_KEY][NUM_SAMPLES_KEY])
            cur_end_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_END_INDEX_KEY])
            cur_begin_index = int(entry[SAMPLING_PARAMETERS_DICT_KEY][SAMPLING_BEGIN_INDEX_KEY])
            # If the current number of samples is not equal to the total count of packets, skip
            #   I need to subtract one because these indices are from MATLAB, so they start at 1
            if cur_num_samples != cur_end_index - (cur_begin_index - 1):
                continue


        top_aoa = entry[TOP_AOAS_LIST_KEY][0]
        true_aoa = float(entry[ANGLE_KEY])
        
        true_distance = entry[DISTANCE_KEY]
        top_location = true_distance * math.sin(top_aoa)
        true_location = true_distance * math.sin(true_aoa)
        error = math.fabs(true_location - top_location)
        errors.append(error)

    errors.sort()
    min_error = errors[0]
    max_error = errors[len(errors) - 1]
    if not suppress_output:
        print("Min Error: %g\nMax Error: %g" % (min_error, max_error))
    return min_error, max_error

def get_num_packets_with_parameters():
    pass

# Run all of the statistics
# Top level, laptop 1 vs laptop 2, 1 m, 2 m
# But constrain it with the number of samples used
def run_statistics(aoa_experiment_comparison_data, distances=None, num_samples=None, separate_laptops=False):
    print("\n\nTop Level Statistics: ")
    error_rate(aoa_experiment_comparison_data, num_samples=num_samples)
    print("")
    mean_error(aoa_experiment_comparison_data, num_samples=num_samples)
    print("")
    median_error(aoa_experiment_comparison_data, num_samples=num_samples)
    print("")
    min_max_error(aoa_experiment_comparison_data, num_samples=num_samples)
    print("")

    # Separate laptops statistics switch
    if separate_laptops:
        print("\n\nLaptop 1 Statistics: ")
        error_rate(aoa_experiment_comparison_data, laptop_id=1, num_samples=num_samples)
        print("")
        mean_error(aoa_experiment_comparison_data, laptop_id=1, num_samples=num_samples)
        print("")
        median_error(aoa_experiment_comparison_data, laptop_id=1, num_samples=num_samples)
        print("")
        min_max_error(aoa_experiment_comparison_data, laptop_id=1, num_samples=num_samples)
        print("")

        print("\n\nLaptop 2 Statistics: ")
        error_rate(aoa_experiment_comparison_data, laptop_id=2, num_samples=num_samples)
        print("")
        mean_error(aoa_experiment_comparison_data, laptop_id=2, num_samples=num_samples)
        print("")
        median_error(aoa_experiment_comparison_data, laptop_id=2, num_samples=num_samples)
        print("")
        min_max_error(aoa_experiment_comparison_data, laptop_id=2, num_samples=num_samples)
        print("")

    # Separate distances statistics switch
    if distances is not None:
        for distance in distances:
            print("\n\n%d m Distance Level Statistics: " % distance)
            error_rate(aoa_experiment_comparison_data, distance=distance, num_samples=num_samples)
            print("")
            mean_error(aoa_experiment_comparison_data, distance=distance, num_samples=num_samples)
            print("")
            median_error(aoa_experiment_comparison_data, distance=distance, num_samples=num_samples)
            print("")
            min_max_error(aoa_experiment_comparison_data, distance=distance, num_samples=num_samples)
            print("")

    # Calculate the error rate and other correctness stats
    """
    for distance in [1, 2]:

        print("\nDistance = %d" % distance)
        error_rate(aoa_experiment_comparison_data, distance=distance, num_samples=num_samples)

        for angle in [-20, -10, 0, 10, 20]:
            print("\nDistance = %d, Angle = %d" % (distance, angle))
            error_rate(aoa_experiment_comparison_data, distance=distance, angle=angle, num_samples=num_samples)

    for angle in [-20, -10, 0, 10, 20]:
        print("\nAngle = %d" % angle)
        error_rate(aoa_experiment_comparison_data, angle=angle, num_samples=num_samples)
    """

# Run the distance-based statistics of the statistics and output the results in a LaTeX table
# Top level, laptop 1 vs laptop 2, 1 m, 2 m
# But constrain it with the number of samples used
def run_latex_statistics(aoa_experiment_comparison_data, distances=[1, 2, 3], 
        num_samples=None, separate_laptops=False):

    header_row = "Distances: & "
    top_rows = ["Correct in Top %d: & " % (i + 1) for i in range(5)]
    mean_row = "Mean Error: & " # "Mean Error (m): & "
    median_row = "Median Error: & " # "Median Error (m): & "
    # Create header row for sampling
    if num_samples is not None:
        num_samples_header_row = "Number of Packets Used in SpotFi: & "
        print(num_samples)
        # Loop over distances (top level header)
        for j in range(len(distances)):
            # Loop over num samples (secondary header)
            for i in range(len(num_samples)):
                if num_samples[i] is None:
                    num_samples_header_row += "1025"
                else:
                    num_samples_header_row += str(num_samples[i])
                if i < len(num_samples) - 1 or j < len(distances) - 1:
                    num_samples_header_row += ' & '
                else:
                    num_samples_header_row += ' \\\\'

    # Create table with samples
    if num_samples is not None:
        # Separate distances statistics
        for i in range(len(distances)):
            
            # Header row
            for k in range(len(num_samples)):
                if k == len(num_samples) // 2:
                    header_row += str(distances[i]) 
                    header_row += ' m '
                header_row += ' & '
            if i == (len(distances) - 1):
                header_row += ' \\\\'

            # Separate sampling statistics
            for k in range(len(num_samples)):

                # Compute error statistics
                top_error_rates = error_rate(aoa_experiment_comparison_data, 
                        distance=distances[i], 
                        num_samples=num_samples[k], 
                        suppress_output=True)
                mean_error_distance = mean_error(aoa_experiment_comparison_data, 
                        distance=distances[i], 
                        num_samples=num_samples[k], 
                        suppress_output=True)
                median_error_distance = median_error(aoa_experiment_comparison_data, 
                        distance=distances[i], 
                        num_samples=num_samples[k], 
                        suppress_output=True)

                # Top
                for j in range(len(top_error_rates)):
                    top_rows[j] += "%.1f \\%%" % top_error_rates[j]
                    if i < (len(distances) - 1) or k < (len(num_samples) - 1):
                        top_rows[j] += ' & '
                    else:
                        top_rows[j] += ' \\\\'

                # Mean
                mean_row += "%.3f m" % mean_error_distance
                if i < (len(distances) - 1) or k < (len(num_samples) - 1):
                    mean_row += ' & '
                else:
                    mean_row += ' \\\\'

                # Median
                median_row += "%.3f m" % median_error_distance
                if i < (len(distances) - 1) or k < (len(num_samples) - 1):
                    median_row += ' & '
                else:
                    median_row += ' \\\\'

    # Create table with default packet counts, no sampling
    else:
        # Separate distances statistics switch
        for i in range(len(distances)):

            # Header row
            header_row += str(distances[i]) 
            header_row += ' m '
            if i < len(distances) - 1:
                header_row += ' & '
            else:
                header_row += ' \\\\'

            # Compute error statistics
            top_error_rates = error_rate(aoa_experiment_comparison_data, 
                    distance=distances[i], 
                    num_samples=num_samples, 
                    suppress_output=True)
            mean_error_distance = mean_error(aoa_experiment_comparison_data, 
                    distance=distances[i], 
                    num_samples=num_samples, 
                    suppress_output=True)
            median_error_distance = median_error(aoa_experiment_comparison_data, 
                    distance=distances[i], 
                    num_samples=num_samples, 
                    suppress_output=True)

            # Top
            for j in range(len(top_error_rates)):
                top_rows[j] += "%.1f \\%%" % top_error_rates[j]
                if i < len(distances) - 1:
                    top_rows[j] += ' & '
                else:
                    top_rows[j] += ' \\\\'

            # Mean
            mean_row += "%.3f m" % mean_error_distance
            if i < len(distances) - 1:
                mean_row += ' & '
            else:
                mean_row += ' \\\\'

            # Median
            median_row += "%.3f m" % median_error_distance
            if i < len(distances) - 1:
                median_row += ' & '
            else:
                median_row += ' \\\\'
        
    # Print full latex table
    # Print beginning line (variable number of slots for num_samples case)
    if num_samples is None:
        print("\\begin{tabular}{| c || c | c | c |}")
    else:
        begin_table_string = '\\begin{tabular}{'
        begin_table_string += '|'
        for i in range((len(num_samples) * len(distances)) + 1):
            begin_table_string += ' c |'
            # If we are on the first element (the header) or the final element of a sample chunk, but not the final element
            if (i == 0 or (i % len(num_samples)) == 0) and i != (len(num_samples) * len(distances)):
                begin_table_string += '|'
        begin_table_string += '}'
        print(begin_table_string)
    print('\\hline')

    print(header_row)
    print('\\hline')

    # Print sampling header row
    if num_samples is not None:
        print(num_samples_header_row)
        print('\\hline')
    print('\\hline')


    # Print top count rows
    for row in top_rows:
        print(row)
        print('\\hline')
    print('\\hline')

    # Print mean row
    print(mean_row)
    print('\\hline')
    # Print median row
    print(median_row)
    print('\\hline')

    # End table
    print("\\end{tabular}")

# Main method---------------------------------------------------------------------------------------
if __name__ == '__main__':
    if len(sys.argv) > 1:
        directory_name = sys.argv[1]
    else:
        directory_name = 'lgtm-distance-angle-experiments-output-data'
    
    data_files_data = parse_spotfi_sampling_output(directory_name)
    aoa_experiment_comparison_data = list()
    # Loop over data files
    for data_file_key in sorted(data_files_data):
        # Parse the data file name to extract data from the file-name like aoa, distance, etc
        file_name_data = parse_file_name(data_file_key)
        # Loop over different sampling parameters
        for sampling_parameters_key in sorted(data_files_data[data_file_key]):

            top_aoas = data_files_data[data_file_key][sampling_parameters_key]

            # Get correct angles and angle index (ordered from highest to lowest likelihood)
            matching_indices = compare_true_and_measured_angles(file_name_data[DISTANCE_KEY], \
                    file_name_data[ANGLE_KEY], top_aoas)

            # Extract sampling parameters from key name
            sampling_parameters_dict = parse_sampling_parameters_key(sampling_parameters_key)

            # Append match data to data list so I can run through correct/incorrect stats
            aoa_matches = dict()
            aoa_matches[DISTANCE_KEY] = file_name_data[DISTANCE_KEY]
            aoa_matches[ANGLE_KEY] = file_name_data[ANGLE_KEY]
            aoa_matches[LAPTOP_ID_KEY] = file_name_data[LAPTOP_ID_KEY]
            aoa_matches[AOA_MATCHES_KEY] = matching_indices
            aoa_matches[TOP_AOAS_LIST_KEY] = top_aoas
            aoa_matches[SAMPLING_PARAMETERS_DICT_KEY] = sampling_parameters_dict

            aoa_experiment_comparison_data.append(aoa_matches)

    distances = [1, 2, 3]
    num_samples_list = None
    if directory_name == 'lgtm-distance-angle-experiments-sampling-output-data' or directory_name == 'lgtm-distance-angle-experiments-sampling-output-data/':
        num_samples_list = [None, 750, 500, 250, 100, 50, 25, 10]
        for num_samples in num_samples_list:
            if num_samples is None:
                print("\n\n\nFor using ALL packets")
            else:
                print("\n\n\nFor sampling %d packets" % num_samples)
            run_statistics(aoa_experiment_comparison_data, 
                    distances=distances, 
                    num_samples=num_samples, 
                    separate_laptops=False)

        run_latex_statistics(aoa_experiment_comparison_data, 
                distances=[1], 
                num_samples=num_samples_list, 
                separate_laptops=False)
        run_latex_statistics(aoa_experiment_comparison_data, 
                distances=[2], 
                num_samples=num_samples_list, 
                separate_laptops=False)
        run_latex_statistics(aoa_experiment_comparison_data, 
                distances=[3], 
                num_samples=num_samples_list, 
                separate_laptops=False)
    else:
        run_statistics(aoa_experiment_comparison_data, 
                distances=distances, 
                num_samples=None,
                separate_laptops=False)

        run_latex_statistics(aoa_experiment_comparison_data, 
                distances=distances, 
                num_samples=None, 
                separate_laptops=False)
        
