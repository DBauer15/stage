#include "test_common.h"

TEST(Buffer, CreateFromVector) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    Buffer buf(data);

    EXPECT_EQ(buf.size(), data.size());
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(buf.data()[i], data[i]);
    }
}

TEST(Buffer, CreateFromPointer) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    Buffer buf(data.data(), data.size());

    EXPECT_EQ(buf.size(), data.size());
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(buf.data()[i], data[i]);
    }
}

TEST(Buffer, SetDataFromVector) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    Buffer buf;
    buf.data(data);

    EXPECT_EQ(buf.size(), data.size());
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(buf.data()[i], data[i]);
    }
}

TEST(Buffer, SetDataFromBlob) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    Buffer buf;
    buf.data(data.data(), data.size());

    EXPECT_EQ(buf.size(), data.size());
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(buf.data()[i], data[i]);
    }
}

TEST(BufferView, FromBuffer) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(data);

    BufferView<uint8_t> view(buf, 0, buf->size());

    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(data[i], view[i]);
    }
}

TEST(BufferView, FromVector) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>();

    BufferView<uint8_t> view(buf, 0, 0);
    view.push_back(data);

    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(data[i], view[i]);
    }
}

TEST(BufferView, FromScalar) {
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>();

    BufferView<uint8_t> view(buf, 0, 0);
    for (auto& d : data)
        view.push_back(d);

    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(data[i], view[i]);
    }
}

TEST(BufferView, OffsetRead) {
    size_t offset = 128;
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(data);

    BufferView<uint8_t> view(buf, offset, buf->size()-offset);

    for (int i = offset; i < 1024; i++) {
        EXPECT_EQ(data[i], view[i-offset]);
    }
}

TEST(BufferView, OffsetWrite) {
    size_t offset = 256;
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(data);

    BufferView<uint8_t> view(buf, offset, buf->size()-offset);
    for(int i = 0; i < offset; i++) {
        view.push_back(data[i]);
    }

    EXPECT_EQ(view.sizeInBytes(), buf->size()-offset);
    for (int i = offset; i < 1024; i++) {
        EXPECT_EQ(data[i], view[i]);
    }
}

TEST(BufferView, StrideRead) {
    size_t stride = 4 * sizeof(uint8_t);
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(data);

    BufferView<uint8_t> view(buf, 0, buf->size(), stride);

    for (int i = 0; i < 1024 / 4; i++) {
        EXPECT_EQ(data[i*4], view[i]);
    }
}

TEST(BufferView, StrideWrite) {
    size_t stride = 4 * sizeof(uint8_t);
    std::vector<uint8_t> data = make_data_array<uint8_t>(1024);
    std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(data);

    BufferView<uint8_t> view(buf, 0, buf->size(), stride);
    for (int i = 0; i < 256; i++) {
        view.push_back(i);
    }

    EXPECT_EQ(view.sizeInBytes(), buf->size() / 4);

    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(i, view[i]);
    }
}