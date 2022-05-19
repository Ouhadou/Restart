#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <stdlib.h>
#include <vector>
#include <utility>
#include "encoder_decoder.h"
//#include "/home/fadoua/eclipse-workspace/Bachelorarbeit/ns-3.35/scratch/Start/encoder_decoder.h"

#include "/home/fadoua/libRaptorQ/src/RaptorQ/RaptorQ_v1_hdr.hpp"


namespace RaptorQ = RaptorQ__v1;


const uint16_t symbol_size = 4; // bytes
float drop_probability = 20.0;
const uint8_t overhead = 4;
std::mt19937_64 rnd;
RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;
using symbol_id = uint32_t; // just a better name
//std::vector<std::pair<symbol_id, std::vector<uint8_t>>> received;
using std::cout; using std::endl;
const uint32_t mysize = 300; 
std::vector<uint8_t> output(mysize, 0); 



//bool kodierer( std::vector<uint8_t> input, const uint32_t mysize);
//bool decoder(std::vector<uint8_t> input, const uint32_t mysize);

bool kodierer(const uint32_t mysize)
{   
    // get a random number generator
    std::ifstream rand("/dev/urandom");
    uint64_t seed = 0;
    rand.read(reinterpret_cast<char *>(&seed), sizeof(seed));
    rand.close();
    rnd.seed(seed);

    RaptorQ__v1::local_cache_size(5000000);
    std::vector<std::pair<symbol_id, std::vector<uint8_t>>> received;

    // the actual input.
    std::vector<uint8_t> input;
    input.reserve(mysize);

    
    std::uniform_int_distribution<int16_t> distr(0,
                                                 std::numeric_limits<uint8_t>::max());

    // fill our input with random data
    for (size_t idx = 0; idx < mysize; ++idx)
    {
        input.push_back(static_cast<uint8_t>(distr(rnd)));
    }
    auto min_symbols = (input.size() * sizeof(uint8_t)) / symbol_size;
  

    if ((input.size() * sizeof(uint8_t)) % symbol_size != 0)
        ++min_symbols;

    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;
    for (auto blk : *RaptorQ::blocks)
    {
        if (static_cast<uint16_t>(blk) >= min_symbols)
        {
            block = blk;
            break;
        }
    }

    //float drop_probability = 20.0;
    RaptorQ::Encoder<typename std::vector<uint8_t>::iterator,
                     typename std::vector<uint8_t>::iterator>
        enc(
            block, symbol_size);

    // give the input to the encoder. the encoder answers with the size of what
    // it can use

    if (enc.set_data(input.begin(), input.end()) != mysize)
    {
        std::cout << "Could not give data to the encoder :(\n";
        return false;
    }
    // actual symbols. you could just use static_cast<uint16_t> (blok)
    // but this way you can actually query the encoder.
    uint16_t _symbols = enc.symbols();
    // print some stuff in output
    std::cout << "Size: " << mysize << " symbols: " << static_cast<uint32_t>(_symbols) << " symbol size: " << static_cast<int32_t>(enc.symbol_size()) << "\n";

    if (!enc.compute_sync())
    {
        // if this happens it's a bug in the library.
        // the **Decoder** can fail, but the **Encoder** can never fail.
        std::cout << "Enc-RaptorQ failure! really bad!\n";
        return false;
    }
    // the probability that a symbol will be dropped.
    if (drop_probability > static_cast<float>(90.0))
        drop_probability = 90.0; // this is still too high probably.

    // we will store here all encoded and transmitted symbols
    // std::pair<symbol id (esi), symbol data>

    {
        // in this block we will generate the symbols that will be sent to
        // the decoder.
        // a block of size X will need at least X symbols to be decoded.
        // we will randomly drop some symbols, but we will keep generating
        // repari symbols until we have the required number of symbols.

        std::uniform_real_distribution<float> drop_rnd(0.0, 100.0);
        uint32_t received_tot = 0;

        // Now get the source symbols.
        // source symbols are specials because they contain the input data
        // as-is, so if you get all of these, you don't need repair symbols
        // to make sure that we are using the decoder, drop the first
        // source symbol.
        auto source_sym_it = enc.begin_source();
        ++source_sym_it; // ignore the first soure symbol (=> drop it)
        source_sym_it++;
        for (; source_sym_it != enc.end_source(); ++source_sym_it)
        {
            // we save the symbol here:
            // make sure the vector has enough space for the symbol:
            // fill it with zeros for the size of the symbol
            std::vector<uint8_t> source_sym_data(symbol_size, 0);

            // save the data of the symbol into our vector
            auto it = source_sym_data.begin();
            auto written = (*source_sym_it)(it, source_sym_data.end());
            if (written != symbol_size)
            {
                // this can only happen if "source_sym_data" did not have
                // enough space for a symbol (here: never)
                std::cout << written << "-vs-" << symbol_size << " Could not get the whole source symbol!\n";
                return false;
            }

            // can we keep this symbol or do we randomly drop it?
            float dropped = drop_rnd(rnd);
            if (dropped <= drop_probability)
            {
                continue; // start the cycle again
            }

            // good, the symbol was received.
            ++received_tot;
            // add it to the vector of received symbols
            symbol_id tmp_id = (*source_sym_it).id();
            received.emplace_back(tmp_id, std::move(source_sym_data));
        }

        std::cout << "Source Packet lost: " << enc.symbols() - received.size()
                  << "\n";

        //--------------------------------------------
        // we finished working with the source symbols.
        // now we need to transmit the repair symbols.
        auto repair_sym_it = enc.begin_repair();
        auto max_repair = enc.max_repair(); // RaptorQ can theoretically handle
                                            // infinite repair symbols
                                            // but computers are not so infinite

        // we need to have at least enc.symbols() + overhead symbols.
        for (; received.size() < (long unsigned int)(enc.symbols() + overhead) &&
               repair_sym_it != enc.end_repair(max_repair);
             ++repair_sym_it)
        {
           
            std::vector<uint8_t> repair_sym_data(symbol_size, 0);

            // save the data of the symbol into our vector
            auto it = repair_sym_data.begin();
            auto written = (*repair_sym_it)(it, repair_sym_data.end());
            if (written != symbol_size)
            {
                // this can only happen if "repair_sym_data" did not have
                // enough space for a symbol (here: never)
                std::cout << written << "-vs-" << symbol_size << " Could not get the whole repair symbol!\n";
                return false;
            }

            // can we keep this symbol or do we randomly drop it?
            float dropped = drop_rnd(rnd);
            if (dropped <= drop_probability)
            {
                continue; // start the cycle again
            }

            // good, the symbol was received.
            ++received_tot;
            // add it to the vector of received symbols
            symbol_id tmp_id = (*repair_sym_it).id();
            received.emplace_back(tmp_id, std::move(repair_sym_data));
        }
        if (repair_sym_it == enc.end_repair(enc.max_repair()))
        {
          
            std::cout << "Maybe losing " << drop_probability << "% is too much?\n";
            return false;
        }
    }
 

    using Decoder_type = RaptorQ::Decoder<
        typename std::vector<uint8_t>::iterator,
        typename std::vector<uint8_t>::iterator>;
    Decoder_type dec(block, symbol_size, Decoder_type::Report::COMPLETE);
    
    //std::vector<uint8_t> output(mysize, 0); 

    // now push every received symbol into the decoder
    for (auto &rec_sym : received)
    {

        symbol_id tmp_id = rec_sym.first;
        auto it = rec_sym.second.begin();
        auto err = dec.add_symbol(it, rec_sym.second.end(), tmp_id);
        if (err != RaptorQ::Error::NONE && err != RaptorQ::Error::NOT_NEEDED)
        {
     
            std::cout << "error adding?\n";
            return false;
        }
    }

    
    dec.end_of_input(RaptorQ::Fill_With_Zeros::NO);
    
    auto res = dec.wait_sync();
    if (res.error != RaptorQ::Error::NONE)
    {
        std::cout << "Couldn't decode.\n";
        return false;
    }

    // now save the decoded data in our output
    size_t decode_from_byte = 0;
    size_t skip_bytes_at_begining_of_output = 0;
    auto out_it = output.begin();
    auto decoded = dec.decode_bytes(out_it, output.end(), decode_from_byte,
                                    skip_bytes_at_begining_of_output);


    if (decoded.written != mysize)
    {
        if (decoded.written == 0)
        {
            // we were really unlucky and the RQ algorithm needed
            // more symbols!
            std::cout << "Couldn't decode, RaptorQ Algorithm failure. "
                         "Can't Retry.\n";
        }
        else
        {
            // probably a library error
            std::cout << "Partial Decoding? This should not have happened: " << decoded.written << " vs " << mysize << "\n";
        }
        return false;
    }
    else
    {
        std::cout << "Decoded: " << mysize << "\n";
    }

    for (uint64_t i = 0; i < mysize; ++i)
    {
        if (input[i] != output[i])
        {
            // this is a bug in the library, please report
            std::cout << "The output does not correspond to the input!\n";
            return false;
        }
    }

    return true;
} 
std::vector<uint8_t> decoded_output()
{ 
    return output; 
} 



