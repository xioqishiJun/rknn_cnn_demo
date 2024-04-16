/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "rknn_api.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <map>

/*-------------------------------------------
                  Functions
-------------------------------------------*/
/*!
 * @brief 加载模型
 * @param filename 模型的路径
 * @param model_size 模型的大小
 * @return
 */
static unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char *) malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(model, 1, model_len, fp))
    {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if (fp)
    {
        fclose(fp);
    }
    return model;
}

/*-------------------------------------------
                  Main Function
-------------------------------------------*/
int main(int argc, char **argv)
{

    // 0-25 -> A-Z 映射 显示结果方便查看
    std::map<int, char> word_dict{
            {  0 , 'A'}
            , {1 , 'B'}
            , {2 , 'C'}
            , {3 , 'D'}
            , {4 , 'E'}
            , {5 , 'F'}
            , {6 , 'G'}
            , {7 , 'H'}
            , {8 , 'I'}
            , {9 , 'J'}
            , {10, 'K'}
            , {11, 'L'}
            , {12, 'M'}
            , {13, 'N'}
            , {14, 'O'}
            , {15, 'P'}
            , {16, 'Q'}
            , {17, 'R'}
            , {18, 'S'}
            , {19, 'T'}
            , {20, 'U'}
            , {21, 'V'}
            , {22, 'W'}
            , {23, 'X'}
            , {24, 'Y'}
            , {25, 'Z'}
    };

    rknn_context ctx = 0;
    int ret;
    int model_len = 0;
    unsigned char *model = nullptr;

    // >>>>> 此处根据芯片型号更改模型
    const char *model_path = "../model/az_handwriting_3566.rknn";
    // <<<<< 此处根据芯片型号更改模型

    // 图片路径
    const char *img_path = "../model/m.jpg";

    // step1: 加载图片
    cv::Mat orig_img = imread(img_path, cv::IMREAD_GRAYSCALE);
    if (!orig_img.data)
    {
        printf("cv::imread %s fail!\n", img_path);
        return -1;
    }
    cv::Mat grayImage = orig_img.clone();
    cv::Mat img_thresh;
    cv::threshold(grayImage, img_thresh, 100, 255, cv::THRESH_BINARY_INV);
    // 图片尺寸变换到  28*28  因为模型的输入是 28*28
    cv::resize(img_thresh, img_thresh, cv::Size(28, 28), 0, 0, cv::INTER_CUBIC);
    cv::Mat img = img_thresh.clone();

    // step2: 加载模型
    model = load_model(model_path, &model_len);
    ret = rknn_init(&ctx, model, model_len, 0, nullptr);
    if (ret < 0)
    {
        printf("rknn_init fail! ret=%d\n", ret);
        return -1;
    }

    // step3: 获取模型的输入输出张量数量
    // 这边省略了获取输入输出张量的信息
    rknn_input_output_num io_num;
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC)
    {
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    // step4: 设置输入张量
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = img.cols * img.rows * img.channels() * sizeof(uint8_t);;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = img.data;
    inputs[0].pass_through = 0;
    // 把输入张量设置到模型中，这样模型就可以获取到输入张量了
    ret = rknn_inputs_set(ctx, io_num.n_input, inputs);
    if (ret < 0)
    {
        printf("rknn_input_set fail! ret=%d\n", ret);
        return -1;
    }

    // step5: 运行模型，进行推理
    printf("rknn_run\n");
    ret = rknn_run(ctx, nullptr);
    if (ret < 0)
    {
        printf("rknn_run fail! ret=%d\n", ret);
        return -1;
    }

    // step6: 获取输出张量
    rknn_output outputs[io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < io_num.n_output; i++)
    {
        outputs[i].want_float = 1;
    }

    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    if (ret < 0)
    {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
    }

    // step7: 打印输出结果
    std::cout << "The results are" << std::endl;

    auto *actions = (float *) (outputs[0].buf);

    for (int i = 0; i < 26; i++)
    {
        std::cout << actions[i] << " ";
    }
    std::cout << std::endl;
    // 找出数组中的最大值索引
    int max_index = std::distance(actions, std::max_element(actions, actions + 26));

    // 使用索引从字典中获取对应的字母
    char max_letter = word_dict[max_index];

    // 输出结果
    std::cout << "The letter with the highest score is: " << max_letter << std::endl;

    // step8: 释放资源
    rknn_outputs_release(ctx, 1, outputs);
    if (ctx > 0)
    {
        rknn_destroy(ctx);
    }
    if (model)
    {
        free(model);
    }
    return 0;
}
