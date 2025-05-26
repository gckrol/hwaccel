#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

class Tensor {
public:
    float *data;
    int hdim;
    int vdim;

    Tensor(int hdim, int vdim) : hdim(hdim), vdim(vdim) {
        data = new float[hdim * vdim];
        if (data == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    ~Tensor() {
        delete[] data;
        data = NULL;
    }
};

static void matmul(const Tensor *matrix, const Tensor *input, Tensor *output) {
    assert(matrix != NULL);
    assert(input != NULL);
    assert(output != NULL);
    assert(input->hdim == matrix->hdim);
    assert(output->hdim == matrix->vdim);

    if (matrix->vdim != input->hdim) {
        fprintf(stderr, "Matrix and input dimensions do not match for multiplication\n");
        exit(1);
    }

    for (int i = 0; i < matrix->hdim; i++) {
        for (int j = 0; j < input->vdim; j++) {
            for (int k = 0; k < matrix->vdim; k++) {
                output->data[i * output->vdim + j] += matrix->data[i * matrix->vdim + k] * input->data[k];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // Create test input data and a test matrix.
    Tensor matrix(3, 3);
    for (int i = 0; i < matrix.hdim; i++) {
      for (int j = 0; j < matrix.vdim; j++) {
        matrix.data[i * matrix.vdim + j] = (float)(i * matrix.vdim + j);
      }
    }
    Tensor input(3, 1);
    for (int i = 0; i < input.hdim; i++) {
        input.data[i] = (float)i;
    }
    Tensor output(3, 1);
    for (int i = 0; i < output.hdim; i++) {
        output.data[i] = 0.0f; // Initialize output to zero
    }
    // Perform matrix multiplication.
    matmul(&matrix, &input, &output);
    // Print the output.
    printf("Output:\n");
    for (int i = 0; i < output.hdim; i++) {
        printf("%f\n", output.data[i]);
    }

    return 0;
}
