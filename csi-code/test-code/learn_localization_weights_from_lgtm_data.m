function learn_localization_weights_from_lgtm_data
    clc
    % Order of elements in X is: Cp, \sigma_{AoA}, \sigma_{ToF}, \mu_{ToF}

    Y_left_desk = zeros(size(prep_Y, 1), 1);
    for ii = 1:size(prep_Y, 1)
        temp = prep_Y(ii, 1) - prep_Y(ii, 2);
        if temp == 0
            Y_left_desk(ii, 1) = 1;
        else
            Y_left_desk(ii, 1) = 0;
        end
    end
    svm_model = fitcsvm(X, Y_left_desk)
    svm_model.Beta
    svm_model.KernelParameters.Scale
    svm_model.Bias
end