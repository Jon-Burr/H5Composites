
#include "H5Composites/MergeUtils.h"
#include "H5Composites/MergeFactory.h"

template <> H5COMPOSITES_REGISTER_TYPE(H5Composites::Plus);
template <> H5COMPOSITES_REGISTER_MERGE(H5Composites::Plus);

template <> H5COMPOSITES_REGISTER_TYPE(H5Composites::Multiplies);
template <> H5COMPOSITES_REGISTER_MERGE(H5Composites::Multiplies);

template <> H5COMPOSITES_REGISTER_TYPE(H5Composites::BitOr);
template <> H5COMPOSITES_REGISTER_MERGE(H5Composites::BitOr);

template <> H5COMPOSITES_REGISTER_TYPE(H5Composites::BitAnd);
template <> H5COMPOSITES_REGISTER_MERGE(H5Composites::BitAnd);