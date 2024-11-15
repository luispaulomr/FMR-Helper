#include "CModGame.h"

#include <iomanip>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "Cards.h"

CModGame::CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name)
{
	auto handle_process = std::make_unique<CHandleProcess>(str_window_name, str_exe_name);

	m_pHandleProcess = std::move(handle_process);

	if (!m_pHandleProcess) {
		std::cout << "[CModGame::CModGame] ERROR: Failed to create CModGame::CModGame object." << "\n";
		return;
	}

	if (!m_pHandleProcess->AttachToProcess()) {
		std::cout << "[CModGame::CModGame] ERROR: Call to CHandleProcess::AttachToProcess failed." << "\n";
		return;
	}

	if (!_LoadGameData()) {
		std::cout << "[CModGame::CModGame] ERROR: Could not load game data." << "\n";
	}
}

bool CModGame::RetryAttach()
{
	if (!m_pHandleProcess) {
		std::cout << "[CModGame::RetryAttach] ERROR: Trying to access an invalid pointer.." << "\n";
		return false;
	}

	if (!m_pHandleProcess->AttachToProcess()) {
		std::cout << "[CModGame::RetryAttach] ERROR: Call to CHandleProcess::AttachToProcess failed." << "\n";
		return false;
	}

	if (!_LoadGameData()) {
		std::cout << "[CModGame::CModGame] ERROR: Could not load game data." << "\n";
	}

	return true;
}

bool CModGame::IsAttached() const
{
	if (!m_pHandleProcess) {
		std::cout << "[CModGame::IsGameRunning] ERROR: Trying to access an invalid pointer." << "\n";
		return false;
	}

	return m_pHandleProcess->IsAttached();
}

std::vector<uint8_t> CModGame::_ReadData(GameConsts_t gd) const
{
	return m_pHandleProcess->ReadMemoryFromBaseAddr(gd.offset, gd.len);
}

uint16_t CModGame::_GetEnemyHealth() const
{
	auto data = _ReadData(GAME_CONSTS[I_ENEMY_HEALTH]);

	uint16_t health = 0;
	std::memcpy(&health, data.data(), data.size());

	return health;
}

bool CModGame::_ReadBinFile
	(
	std::string path_file,
	size_t offset,
	size_t len,
	std::vector<BYTE>& buf
	) const
{
	if (!buf.size()) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Buf size is 0." << "\n";
		return false;
	}

	if (path_file.empty()) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Empty path for binary file." << "\n";
		return false;
	}

	std::ifstream in_file(path_file, std::ios_base::in | std::ios_base::binary);

	if (!in_file) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Could not open binary file for reading." << "\n";
		return false;
	}

	in_file.seekg(offset, std::ios::beg);
	in_file.read(reinterpret_cast<char*>(buf.data()), len);
	
	if (len != in_file.gcount()) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Could not read from binary file." << "\n";
		return false;
	}

	return true;
}

uint32_t CModGame::_Get32bitColor(uint16_t clut) const
{

	uint32_t red = (clut & 0x001f) * 8;
	uint32_t green = ((clut & 0x03e0) >> 5) * 8;
	uint32_t blue = ((clut & 0x7c00) >> 10) * 8;

	uint32_t ret = 0x00;

	/* 
	 * red:   0111 1100 0000 0000
	 * green: 0000 0011 1110 0000
	 * blue:  0000 0000 0001 1111
	 */

	ret = (static_cast<uint32_t>(red) << 16) & 0x00ff0000;
	ret |= ((static_cast<uint32_t>(green) << 8) & 0x0000ff00);
	ret |= ((static_cast<uint32_t>(blue)) & 0x000000ff);

	return ret;
}

uint32_t CModGame::_CLUTColor32bit(const std::vector<uint8_t>& cluts, int index) const
{

	index *= 2;

	uint16_t clut = static_cast<uint16_t>(cluts[index]) | (static_cast<uint16_t>(cluts[index + 1]) << 8);

	return _Get32bitColor(clut);
}

size_t CModGame::_GetBMPHeaderLen() const
{
	return (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
}

void CModGame::_GetBMPHeader(std::vector<BYTE>& header,
							 size_t width,
							 size_t height,
							 size_t bpp,
						     size_t len_data,
							 size_t len_header) const
{
	BITMAPFILEHEADER bfh = { 0 };

	// This value should be values of BM letters i.e 0x4D42  
	// 0x4D = M 0×42 = B storing in reverse order to match with endian  
	bfh.bfType = 0x4D42;
	//bfh.bfType = 'B'+('M' << 8); 

	// <<8 used to shift ‘M’ to end  */  

	// Offset to the RGBQUAD  
	bfh.bfOffBits = len_header;

	// Total size of image including size of headers  
	bfh.bfSize = len_header + len_data;

	BITMAPINFOHEADER bmpInfoHeader = { 0 };

	// Set the size  
	bmpInfoHeader.biSize = sizeof(bmpInfoHeader);

	// Bit count  
	bmpInfoHeader.biBitCount = bpp * 8;

	// Use all colors  
	bmpInfoHeader.biClrImportant = 0;

	// Use as many colors according to bits per pixel  
	bmpInfoHeader.biClrUsed = 0;

	// Store as un Compressed  
	bmpInfoHeader.biCompression = BI_RGB;

	// Set the height in pixels  
	bmpInfoHeader.biHeight = height;

	// Width of the Image in pixels  
	bmpInfoHeader.biWidth = width;

	// Default number of planes  
	bmpInfoHeader.biPlanes = 1;

	// Calculate the image size in bytes  
	bmpInfoHeader.biSizeImage = len_data;

	bmpInfoHeader.biXPelsPerMeter = 0;

	bmpInfoHeader.biYPelsPerMeter = 0;

	bmpInfoHeader.biClrUsed = 0;

	bmpInfoHeader.biClrImportant = 0;

	::memcpy(header.data(), &bfh, sizeof(bfh));
	::memcpy(header.data() + sizeof(bfh), &bmpInfoHeader, sizeof(bmpInfoHeader));
}

void CModGame::_TIMtoBMP(const std::vector<BYTE>& data,
						const std::vector<BYTE>& clut,
						std::vector<BYTE>& image,
						size_t width,
						size_t height) const
{
	
	size_t len_header = _GetBMPHeaderLen();
	size_t len_data = SMALL_IMAGE_WIDTH * SMALL_IMAGE_HEIGHT * SMALLIMAGE_BPP;

	image.resize(len_header + len_data);

	_GetBMPHeader(image, SMALL_IMAGE_WIDTH, SMALL_IMAGE_HEIGHT, SMALLIMAGE_BPP, len_data, len_header);

	auto it_data = data.begin();
	auto p_image = image.data() + len_header;

	for (uint32_t i = 0; i < height; ++i) {
		for (uint32_t j = 0; j < width; ++j) {

			uint32_t pixel = _CLUTColor32bit(clut, *it_data);

			uint32_t y = (height - 1 - i) * width;
			uint32_t x = j;
			uint32_t xy = (x + y) * 4;

			//if (x + y >= image.size()) {
			//	continue;
			//}

			p_image[xy + 3] = 0x00;
			p_image[xy + 2] = (pixel >> 16) & 0xff;
			p_image[xy + 1] = (pixel >> 8) & 0xff;
			p_image[xy] = pixel & 0xff;

			++it_data;
		}
	}
}

#include <sstream>

std::string int_to_hex(BYTE i)
{
	std::stringstream stream;
	stream << "0x"
		<< std::setfill('0') << std::setw(2)
		<< std::hex << i;
	return stream.str();
}

bool CModGame::_LoadSmallImages(std::vector<std::vector<BYTE>>& images,
							    const std::vector<std::vector<Card_t>>& fusions) const
{
	images.resize(MAX_CARDS);
	size_t len_to_read = LEN_TOTAL_IMAGES;

	std::vector<BYTE> tmp(len_to_read);

	if (!_ReadBinFile(m_path_bin_file, BIN_FILE_IMAGES_OFFSET, len_to_read, tmp)) {
		std::cout << "[CModGame::_LoadSmallImages] ERROR: Could not read binary file." << "\n";
		images.resize(0);
		return false;
	}

	/* totally empirical data */

	size_t len_data = 2048;
	size_t num_data = 5054;
	std::vector<BYTE> buf(len_data * num_data);

	for (auto i = 0; i < num_data; ++i) {
		auto offset = i * len_data;
		auto skip = i * 304;
		std::memcpy(buf.data() + offset, tmp.data() + offset + skip, len_data);
	}

	std::vector<BYTE> image_data(LEN_DATA_SMALL_IMAGE);
	std::vector<BYTE> image_clut(LEN_CLUT_SMALL_IMAGE);

	for (auto i = 0; i < images.size(); ++i) {
		auto addr_to_read = buf.begin() + LEN_TOTAL_IMAGE * i + BIN_FILE_SMALL_IMAGES_INC;

		std::copy(addr_to_read,
				  addr_to_read + image_data.size(),
				  image_data.data());

		addr_to_read += LEN_DATA_SMALL_IMAGE;

		std::copy(addr_to_read,
				  addr_to_read + image_clut.size(),
				  image_clut.data());

		_TIMtoBMP(image_data, image_clut, images[i], SMALL_IMAGE_WIDTH, SMALL_IMAGE_HEIGHT);
	}

	return true;
}

bool CModGame::_LoadFusions(std::vector<std::vector<Card_t>>& fusions) const
{
	fusions.resize(MAX_CARDS);
	size_t len_to_read = LEN_TOTAL_FUSIONS;
	std::vector<BYTE> tmp(len_to_read);

	if (!_ReadBinFile(m_path_bin_file, BIN_FILE_FUSIONS_OFFSET, len_to_read, tmp)) {
		std::cout << "[CModGame::_LoadFusions] ERROR: Could not read binary file." << "\n";
		fusions.resize(0);
		return false;
	}

	/* totally empirical data */

	size_t len_data = 2048;
	size_t num_data = 32;
	std::vector<BYTE> buf(len_data * num_data);

	for (auto i = 0; i < num_data; ++i) {
		auto offset = i * len_data;
		auto skip = i * 304;
		std::memcpy(buf.data() + offset, tmp.data() + offset + skip, len_data);
	}

	for (auto i = 0; i < fusions.size(); ++i) {
		uint16_t pos_to_read = 0;
		std::memcpy(&pos_to_read, buf.data() + 2 + i * 2, 2);

		if (!pos_to_read) {
			continue;
		}

		uint8_t num_of_fusions = 0;
		std::memcpy(&num_of_fusions, buf.data() + pos_to_read, 1);
		pos_to_read++;

		if (!num_of_fusions) {
			std::memcpy(&num_of_fusions, buf.data() + pos_to_read, 1);
			pos_to_read++;
			num_of_fusions = 511 - num_of_fusions;
		}

		while (num_of_fusions > 0) {
			BYTE bytes_to_read[5] = { 0 };
			uint32_t bytes_fusion[4] = { 0 };

			std::memcpy(bytes_to_read, buf.data() + pos_to_read, sizeof(bytes_to_read));
			pos_to_read += sizeof(bytes_to_read);

			bytes_fusion[0] = (bytes_to_read[0] & 3) << 8 | bytes_to_read[1];
			bytes_fusion[1] = (bytes_to_read[0] >> 2 & 3) << 8 | bytes_to_read[2];
			bytes_fusion[2] = (bytes_to_read[0] >> 4 & 3) << 8 | bytes_to_read[3];
			bytes_fusion[3] = (bytes_to_read[0] >> 6 & 3) << 8 | bytes_to_read[4];

			/* TODO: refactor a little bit */

			{
				Card_t card;

				card.cards.push_back(static_cast<uint16_t>(i));
				card.cards.push_back(static_cast<uint16_t>(bytes_fusion[0]) - 1);
				card.card = static_cast<uint16_t>(bytes_fusion[1]) - 1;

				fusions[i].push_back(card);
			}

			--num_of_fusions;

			if (num_of_fusions > 0) {

				{
					Card_t card;

					card.cards.push_back(static_cast<uint16_t>(i));
					card.cards.push_back(static_cast<uint16_t>(bytes_fusion[2]) - 1);
					card.card = static_cast<uint16_t>(bytes_fusion[3]) - 1;

					fusions[i].push_back(card);
				}

				--num_of_fusions;
			}
		}
	}

	return true;
}

bool CModGame::_LoadCards(std::vector<CardData_t>& cards) const
{
	cards.resize(MAX_CARDS);
	size_t len_to_read = 10000;
	std::vector<BYTE> buf(len_to_read);

	if (!_ReadBinFile(m_path_bin_file, BIN_FILE_CARDS_OFFSET, len_to_read, buf)) {
		std::cout << "[CModGame::_LoadCards] ERROR: Could not read binary file." << "\n";
		cards.resize(0);
		return false;
	}

	for (auto i = 0; i < cards.size(); ++i) {
		uint32_t num = 0;
		size_t inc = 0;

		/* totally empirical data */
		if (i > 366) {
			inc = 304;
		}

		auto pos_to_read = buf.data() + i * BIN_FILE_CARDS_INC + inc;
		std::memcpy(&num, pos_to_read, sizeof(num));

		cards[i].atk = (num & 0x1ff) * 10;
		cards[i].def = ((num >> 9) & 0x1ff) * 10;
	}
}

bool CModGame::_LoadGameData()
{
	if (m_path_bin_file.empty()) {
		m_path_bin_file = _GetPathBinFile();

		if (m_path_bin_file.empty()) {
			std::cout << "[CModGame::LoadGameData] ERROR: Could not get path to binary file." << "\n";
			return false;
		}
	}

	if (m_small_images.size() && m_fusions.size()) {
		return true;
	}

	if (!_LoadFusions(m_fusions)) {
		std::cout << "[CModGame::LoadGameData] ERROR: Could not load fusions from binary file." << "\n";
		return false;
	}

	if (!_LoadSmallImages(m_small_images, m_fusions)) {
		std::cout << "[CModGame::LoadGameData] ERROR: Could not load small images from binary file." << "\n";
		return false;
	}

	if (!_LoadCards(m_cards)) {
		std::cout << "[CModGame::LoadGameData] ERROR: Could not load cards from binary file." << "\n";
		return false;
	}
}

std::vector<uint16_t> CModGame::GetMyHandCards() const
{
	std::vector<uint16_t> cards(MAX_HAND_CARDS);

	auto mem = _ReadData(GAME_CONSTS[I_MY_HAND_CARDS]);

	for (auto i = 0; i < cards.size(); ++i) {
		uint16_t card = 0;
		std::memcpy(&card, mem.data() + 28 * i, 2);
		cards[i] = card - 1;
	}

	return cards;
}

std::vector<uint16_t> CModGame::GetMyTableCards() const
{
	std::vector<uint16_t> cards;

	auto mem = _ReadData(GAME_CONSTS[I_MY_TABLE_CARDS]);

	for (int i = 0; i < MAX_HAND_CARDS; ++i) {
		uint16_t card = 0;
		std::memcpy(&card, mem.data() + 28 * i, 2);

		uint8_t validity = 0;
		std::memcpy(&validity, mem.data() + 28 * i + 11, 1);

		if (validity) {
			cards.push_back(card - 1);
		}
	}

	return cards;
}

std::string CModGame::_GetPathBinFile() const
{
	auto mem = _ReadData(GAME_CONSTS[I_PATH_BIN_FILE]);
	return std::string(mem.begin(), mem.end());
}

bool CModGame::IsDuel() const
{
	return (_GetEnemyHealth() != 0);
}

std::vector<Card_t> CModGame::GetMyFusions()
{
	auto table_cards = GetMyTableCards();
	auto hand_cards = GetMyHandCards();

	auto fusions = GetFusions(table_cards, hand_cards, m_fusions);

	//std::sort(fusions.begin(), fusions.end(), _cmp_cards);

	std::sort(fusions.begin(), fusions.end(), [&cards = std::as_const(m_cards)](const auto& card_1, const auto& card_2) {
		return (cards[card_1.card].atk > cards[card_2.card].atk);
	});

	return fusions;
}

void CModGame::PrintMyFusions(const std::vector<Card_t>& fusions) const
{
	for (auto& fusion : fusions) {
		for (auto i = 0; i < fusion.cards.size(); ++i) {
			std::cout << fusion.uid_cards[i] << "(" << fusion.cards[i] << ")" << " ";
		}
		std::cout << "ATK: " << m_cards[fusion.card].atk;
		std::cout << "(" << fusion.card << ")";
		std::cout << "\n";
	}
}

std::vector<BYTE> CModGame::GetSmallImage(size_t i) const
{
	return m_small_images[i];
}