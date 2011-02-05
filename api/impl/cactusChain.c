/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include "cactusGlobalsPrivate.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic chain functions.
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Chain *chain_construct(Flower *flower) {
    return chain_construct2(cactusDisk_getUniqueID(flower_getCactusDisk(flower)), flower);
}

Chain *chain_construct2(Name name, Flower *flower) {
    Chain *chain;
    chain = st_malloc(sizeof(Chain));
    chain->name = name;
    chain->flower = flower;
    chain->link = NULL;
    chain->linkNumber = 0;
    flower_addChain(flower, chain);
    return chain;
}

void chain_destruct(Chain *chain) {
    flower_removeChain(chain_getFlower(chain), chain);
    if (chain->link != NULL) {
        link_destruct(chain->link);
    }
    free(chain);
}

Link *chain_getLink(Chain *chain, int32_t linkIndex) {
    int32_t i;
    Link *link;

#ifdef BEN_DEBUG
    assert(linkIndex >= 0);
    assert(linkIndex < chain->linkNumber);
#endif

    i = 0;
    link = chain->link;
    while (i++ < linkIndex) {
        link = link->nLink;
    }
    return link;
}

int32_t chain_getLength(Chain *chain) {
    return chain->linkNumber;
}

Block **chain_getBlockChain(Chain *chain, int32_t *blockNumber) {
    int32_t i;
    Link *link;
    End *end;
    Block *block;
    bool circular = 0;
    struct List *blocks = constructEmptyList(0, NULL);
    for (i = 0; i < chain_getLength(chain); i++) {
        link = chain_getLink(chain, i);
        end = link_get3End(link);
        block = end_getBlock(end);
        if (block != NULL) {
            assert(block_getOrientation(block));
            listAppend(blocks, block);
            if(chain_getLength(chain) == 1 && link_get5End(link) == end_getOtherBlockEnd(end)) {
                circular = 1;
            }
        }
    }
    if (chain_getLength(chain) > 0 && !circular) {
        link = chain_getLink(chain, chain_getLength(chain) - 1);
        end = link_get5End(link);
        block = end_getBlock(end);
        if (block != NULL) {
            assert(block_getOrientation(block));
            listAppend(blocks, block);
        }
    }
    i = sizeof(void *) * (blocks->length + 1);
    Block **blockChain = memcpy(st_malloc(i), blocks->list, i);
    *blockNumber = blocks->length;
    destructList(blocks);
    return blockChain;
}

Name chain_getName(Chain *chain) {
    return chain->name;
}

Flower *chain_getFlower(Chain *chain) {
    return chain->flower;
}

double chain_getAverageInstanceBaseLength(Chain *chain) {
    Block **blocks;
    int32_t i, j, l = 0;
    double k = 0.0;
    blocks = chain_getBlockChain(chain, &i);
    l = 0;
    for (j = 0; j < i; j++) {
        k += block_getLength(blocks[j]);
    }
    free(blocks);
    for (i = 0; i < chain_getLength(chain); i++) {
        Flower *nestedFlower = group_getNestedFlower(link_getGroup(chain_getLink(chain,
                i)));
        if (nestedFlower != NULL) {
            k += (flower_getTotalBaseLength(nestedFlower) / flower_getSequenceNumber(
                    nestedFlower));
        }
    }
    return k;
}

bool chain_isCircular(Chain *chain) {
    assert(chain_getLength(chain) > 0);
    End *_3End = link_get3End(chain_getLink(chain, 0));
    End *_5End = link_get5End(chain_getLink(chain, chain_getLength(chain)-1));
    return end_isBlockEnd(_3End) && end_getOtherBlockEnd(_3End) == _5End;
}

void chain_check(Chain *chain) {
    Link *link = NULL, *pLink = NULL;
    int32_t i;
    assert(chain_getLength(chain) > 0);
    for (i = 0; i < chain_getLength(chain); i++) {
        link = chain_getLink(chain, i);
        //That each link is properly contained in the chain.
        assert(chain == link_getChain(link));
        End *_5End = link_get3End(link);
        End *_3End = link_get5End(link);
        assert(_5End != NULL);
        assert(_3End != NULL);
        //Links and the contained ends are properly connected.
        assert(group_getLink(end_getGroup(_5End)) == link);
        assert(group_getLink(end_getGroup(_3End)) == link);
        //Check the orientations
        assert(end_getOrientation(_5End));
        assert(end_getOrientation(_3End));

        //Check stub ends are not free stubs.
        if (end_isStubEnd(_5End)) {
            assert(end_isAttached(_5End));
        }
        if (end_isStubEnd(_3End)) {
            assert(end_isAttached(_3End));
        }

        //That the ends are consistently oriented
        assert(end_getSide(_5End) != end_getSide(_3End));
        assert(!end_getSide(_5End));
        assert(end_getSide(_3End));

        //That each contiguous pair of link groups are bridged by a block.
        if (pLink != NULL) {
            assert(end_isBlockEnd(link_get5End(pLink)));
            assert(end_isBlockEnd(_5End));
            assert(end_getOtherBlockEnd(link_get5End(pLink)) == _5End);
        } else {
            if (end_isBlockEnd(_5End)) {
                //If a block end is at the 5 prime end of a chain the other end of the
                //block is not in a link group (otherwise the chain is not maximal).
                Link *nextLink = group_getLink(end_getGroup(end_getOtherBlockEnd(_5End)));
                assert(nextLink == NULL || link == chain_getLink(chain, 0));
            }
        }
        pLink = link;
    }
    //If a block end is at the 3 prime end of a chain the other end of the
    //block is not in a link group (otherwise the chain is not maximal).
    assert(link != NULL);
    if (end_isBlockEnd(link_get5End(link))) {
        Link *nextLink = group_getLink(end_getGroup(end_getOtherBlockEnd(link_get5End(link))));
        assert(nextLink == NULL || link == chain_getLink(chain, chain_getLength(chain)-1));
    }
}

/*
 * Private functions
 */

void chain_addLink(Chain *chain, Link *childLink) {
    Link *pLink;
    if (chain->linkNumber != 0) {
        pLink = chain_getLink(chain, chain->linkNumber - 1);
        pLink->nLink = childLink;
        childLink->pLink = pLink;
        assert(link_get5End(pLink) != link_get3End(childLink));
        assert(end_getBlock(link_get5End(pLink)) != NULL);
        assert(end_getBlock(link_get3End(childLink)) != NULL);
        assert(end_getBlock(link_get5End(pLink)) == end_getBlock(link_get3End(childLink)));
    } else {
        childLink->pLink = NULL;
        chain->link = childLink;
    }
    childLink->nLink = NULL;
    childLink->linkIndex = chain->linkNumber++;
}

void chain_setFlower(Chain *chain, Flower *flower) {
    flower_removeChain(chain_getFlower(chain), chain);
    chain->flower = flower;
    flower_addChain(flower, chain);
}

void chain_joinP(Chain *chain, stList *list) {
    for(int32_t i=0; i<chain_getLength(chain); i++) {
        Link *link = chain_getLink(chain, i);
        stList_append(list, link_get3End(link));
        stList_append(list, link_get5End(link));
    }
    chain_destruct(chain);
}

void chain_join(Chain *_5Chain, Chain *_3Chain) {
#ifdef BEN_DEBUG
    assert(chain_getLength(_5Chain) > 0);
    assert(chain_getLength(_3Chain) > 0);
    Link *_5Link = chain_getLink(_5Chain, chain_getLength(_5Chain)-1);
    Link *_3Link = chain_getLink(_3Chain, 0);
    End *_5End = link_get5End(_5Link);
    End *_3End = link_get3End(_3Link);
    assert(end_isBlockEnd(_5End));
    assert(end_isBlockEnd(_3End));
    assert(end_getOtherBlockEnd(_5End) == _3End);
    assert(end_getOtherBlockEnd(_3End) == _5End);
    assert(_5Chain != _3Chain); //If this is false and the others are all true then your trying to merge a circular chain with itself.
#endif
    Chain *newChain = chain_construct(chain_getFlower(_5Chain));
    stList *list = stList_construct();
    chain_joinP(_5Chain, list);
    chain_joinP(_3Chain, list);
    for(int32_t i=0; i<stList_length(list); i+=2) {
        End *end1 = stList_get(list, i);
        End *end2 = stList_get(list, i+1);
        Group *group = end_getGroup(end1);
#ifdef BEN_DEBUG
        assert(end_getGroup(end2) == group);
        assert(!end_getSide(end1));
        assert(end_getSide(end2));
        assert(group_getLink(group) == NULL);
#endif
        link_construct(end1, end2, group, newChain);
    }
    stList_destruct(list);
}

/*
 * Serialisation functions.
 */

void chain_writeBinaryRepresentation(Chain *chain, void(*writeFn)(
        const void * ptr, size_t size, size_t count)) {
    Link *link;
    binaryRepresentation_writeElementType(CODE_CHAIN, writeFn);
    binaryRepresentation_writeName(chain_getName(chain), writeFn);
    link = chain_getLink(chain, 0);
    while (link != NULL) {
        link_writeBinaryRepresentation(link, writeFn);
        link = link_getNextLink(link);
    }
    binaryRepresentation_writeElementType(CODE_CHAIN, writeFn);
}

Chain *chain_loadFromBinaryRepresentation(void **binaryString, Flower *flower) {
    Chain *chain;

    chain = NULL;
    if (binaryRepresentation_peekNextElementType(*binaryString) == CODE_CHAIN) {
        binaryRepresentation_popNextElementType(binaryString);
        chain = chain_construct2(binaryRepresentation_getName(binaryString),
                flower);
        while (link_loadFromBinaryRepresentation(binaryString, chain) != NULL)
            ;
        assert(binaryRepresentation_peekNextElementType(*binaryString) == CODE_CHAIN);
        binaryRepresentation_popNextElementType(binaryString);
    }
    return chain;
}

Chain *chain_getStaticNameWrapper(Name name) {
    static Chain chain;
    chain.name = name;
    return &chain;
}