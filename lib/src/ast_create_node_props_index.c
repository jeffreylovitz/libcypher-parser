/* vi:set ts=4 sw=4 expandtab:
 *
 * Copyright 2016, Chris Leishman (http://github.com/cleishm)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../../config.h"
#include "astnode.h"
#include "util.h"
#include <assert.h>


struct create_index
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *label;
    unsigned int nprops;
    const cypher_astnode_t *prop_names[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_schema_command_astnode_vt };

const struct cypher_astnode_vt cypher_create_node_props_index_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "CREATE INDEX",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_create_node_props_index(
        const cypher_astnode_t *label, cypher_astnode_t * const *prop_names,
        unsigned int nprops, cypher_astnode_t **children,
        unsigned int nchildren, struct cypher_input_range range)
{
    REQUIRE_TYPE(label, CYPHER_AST_LABEL, NULL);
    REQUIRE(nprops > 0, NULL);
    REQUIRE_TYPE_ALL(prop_names, nprops, CYPHER_AST_PROP_NAME, NULL);

    struct create_index *node = calloc(1, sizeof(struct create_index) +
            nprops * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode),
            CYPHER_AST_CREATE_NODE_PROPS_INDEX, children, nchildren, range))
    {
        goto cleanup;
    }
    node->label = label;
    memcpy(node->prop_names, prop_names, nprops * sizeof(cypher_astnode_t *));
    node->nprops = nprops;
    return &(node->_astnode);

    int errsv;
cleanup:
    errsv = errno;
    free(node);
    errno = errsv;
    return NULL;
}


const cypher_astnode_t *cypher_ast_create_node_props_index_get_label(
                const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROPS_INDEX, NULL);
    struct create_index *node =
            container_of(astnode, struct create_index, _astnode);
    return node->label;
}


unsigned int cypher_ast_create_node_props_index_nprops(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROPS_INDEX, -1);
    struct create_index *node =
            container_of(astnode, struct create_index, _astnode);
    return node->nprops;
}


const cypher_astnode_t *cypher_ast_create_node_props_index_get_prop_name(
                const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROPS_INDEX, NULL);
    struct create_index *node =
            container_of(astnode, struct create_index, _astnode);
    if (index >= node->nprops)
    {
        return NULL;
    }
    return node->prop_names[index];
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_CREATE_NODE_PROPS_INDEX, -1);
    struct create_index *node =
            container_of(self, struct create_index, _astnode);

    size_t n = 0;
    ssize_t r = snprintf(str, size, "ON=:@%u(", node->label->ordinal);
    if (r < 0)
    {
        return -1;
    }
    n += r;

    for (unsigned int i = 0; i < node->nprops; )
    {
        ssize_t r = snprintf(str+n, (n < size)? size-n : 0,
                "@%u", node->prop_names[i]->ordinal);
        if (r < 0)
        {
            return -1;
        }
        n += r;
        if (++i < node->nprops)
        {
            if (n < size)
            {
                str[n] = ',';
            }
            n++;
            if (n < size)
            {
                str[n] = ' ';
            }
            n++;
        }
    }
    if (n < size)
    {
        str[n] = ')';
    }
    n++;
    return n;
}
