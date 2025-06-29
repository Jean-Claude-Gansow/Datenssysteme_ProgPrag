#ifndef PARTITIONING_MNGR_H
#define PARTITIONING_MNGR_H

#include <vector>
#include <cstdlib>
#include <cstring>
#include "DataTypes.h"
#include "Tokenization_mngr.h"

// Annahme: InType hat operator[](size_t) für Tokenzugriff
template <typename base_type, typename InType, size_t N>
class Partitioning_mngr 
{
public:
    using Tokenizer = Tokenization_mngr<N, base_type, InType>;

    // Wrapper-Funktion: bereitet das Startarray vor
    dataSet<partition> *create_partitions(const dataSet<InType> &input, Tokenizer *tokenizer, const std::vector<category> &class_hierarchy)
    {
        dataSet<partition>* dataSetPartition = new dataSet<Partition>();
        dataSetPartition->size = input.size;
        dataSetPartition->data = new partition[input.size];

        size_t last_inserted_at = 0;
        dataSetPartition->data[0].data = new pair[input.size]; // new partitions always as big as the original one, to prevent an overflow in case of a really polarized dataset
        //partition pair structur: ID, adress of the original tokenized datastructure
        // data[0] is the partition ID, data[1] is the pointer to the original data
        // this is used to keep track of which partition contains which data
        for (int i = 0; i < input.size; i++)
        {
            dataSetPartition->data[0].data[i].data[0] = i;
            dataSetPartition->data[0].data[i].data[1] = &input.data[i]; // copy the original data into the first partition
        }

        dataSet<partition>* split_partitions = this->split_partition_by_hierarchy(dataSetPartition,tokenizer,class_hierarchy,&last_inserted_at,0)

        //insert split partitions into master partition
        for(split_partitions.size)

        return dataSetPartition;
    }

private:
    //storage: list of all partitions, will contain all split up partitions
    //tokenizer: tokenizer, holding all information about how many tokens are in which class
    //class_hierarchy: hierarchy of classes, used to split up the partitions
    //split up index: storage list index of the partition that his call is supposed to split up
    //hierarchy_index: index of the current hierarchy level, used to determine which class to split by
    dataSet<partition>* split_partition_by_hierarchy(dataSet<partition>* storage, Tokenizer *tokenizer, const std::vector<category> &class_hierarchy,size_t* split_up_index, size_t hierarchy_index)
    {
        if(hierarchy_index >= class_hierarchy.size())
        {return;} //no more hierarchy levels to split by

        category current_class = class_hierarchy[hierarchy_index];
        size_t num_split_partitions = tokenizer->m_class_tokens_found[current_class]; 
        partition *tmp_partition_storage = new partition[num_split_partitions]; //TODO: change to dataSet[partition] and change every call to it by tmp_partition_storage->data[<whatever index was used>].<-- use a dot, everything inside is an object
        for (int i = 0; i < num_split_partitions; i++)
        {
            tmp_partition_storage[i].data = new pair[storage->size]; //new partitions always as big as the original one, to prevent an overflow in case of a really polarized dataset
        }
        for(int i = 0; i < storage->size; ++i)
        {
            pair entry = storage->data[split_up_index].data[i]; //get current entry of currently split partition
            token entry_token_val = *(reinterpret_cast<InType*>(entry.data[0]))[current_class]-1; //might not need the dreferencing
            if(entry_token_val == 0)
            {
                for(int j = 0; j < num_split_partitions;++j)
                {
                    tmp_partition_storage[j].data[tmp_partition_storage[j].size++] = entry; //add to every buffer
                }
                continue; //next entry
            }
            tmp_partition_storage[entry_token_val].data[tmp_partition_storage[entry_token_val].size++] = entry; // add only to the 
        }

        //calculate product of all remaining token amounts in hierarchy classes, to size the buffer for it
        class_token_product(tokenizer,class_hierarchy,hierarchy_index);
        dataSet<partition>** further_split_partitions = new dataSet<partition>*[]
        for(int i = 0; i < num_split_partitions; ++i)
        {
            if(tmp_partition_storage->size > this->size_threshhold)
            {
                //create temporary dataset, based on the partition being split
                split_partition_by_hierarchy()
            }
        }
    }
private:
size_t class_token_product(Tokenizer *tkn, const std::vector<category> &class_hierarchy,size_t curclass) 
{
    size_t sum = 1; 
    for(int i = curclass; i < class_hierarchy.size();++i)
    {
        sum *= tkn->m_class_tokens_found[class_hierarchy[i]]; //sum * amount of found token of class
    }
    return sum;
}

private : size_t size_threshhold;
};

#endif // PARTITIONING_MNGR_H