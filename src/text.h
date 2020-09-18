
struct WatermarkText {
#define TEXT_LINE_SIZE 1024
#define TEXT_LINES_COUNT 3
  char lines[TEXT_LINES_COUNT][TEXT_LINE_SIZE] = {0};
};

void watermark_init_font(const char *filename);
void watermark_draw_text(unsigned char *texture, int width, int height,
                         int channels, const WatermarkText &text,
                         float color[4], int letter_height);

