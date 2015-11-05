
#ifndef __MLIB_MACRO_INC__
#define __MLIB_MACRO_INC__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/

/*

��ʾ:
�Ķ����ļ�, ��Ҫһ����������������.

*/


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ģ��ע��궨��
*****************************************************************************/

/* ע��ͨ·����ģ�鼯 */
#define MLIB_REGISTER_PATH_MODULE_BEGIN(profile,path)      \
                    MLIB_MODULE_REG_STRU g_astMLIB_##profile##_##path##_Path[]={\

#define MLIB_REGISTER_PATH_MODULE_END()                      };

/* ע��ģ�� */
#define MLIB_REGISTER_MODULE(id,init,set,get,proc,clear)    \
    {id,init,set,get,proc,clear},

/*****************************************************************************
  2 ģ���ȡ�궨��
*****************************************************************************/

/* ��ȡͨ·ģ���׵�ַ */
#define MLIB_GetPathModules(profile,path)                  \
                    g_astMLIB_##profile##_##path##_Path

/* ��ȡͨ·ģ����� */
#define MLIB_GetPathModuleNum(profile,path)                \
                    sizeof(g_astMLIB_##profile##_##path##_Path)/sizeof(MLIB_MODULE_REG_STRU)


/*****************************************************************************
  3 ����ע��궨��
*****************************************************************************/

#define MLIB_REGISTER_PATH_BEGIN(profile)      \
                    MLIB_PATH_REG_STRU g_astMLIB_##profile##_Paths[]={\

#define MLIB_REGISTER_PATH_END()                      };

/* ע�᳡�� */
#define MLIB_REGISTER_PATH(id,modules,num)    \
    {id,modules,num},

/*****************************************************************************
  3 ������ȡ�궨��
*****************************************************************************/

/* ��ȡͨ���׵�ַ */
#define MLIB_GetPaths(profile)                  \
                    g_astMLIB_##profile##_Paths

/* ��ȡͨ������ */
#define MLIB_GetPathNum(profile)                \
                    sizeof(g_astMLIB_##profile##_Paths)/sizeof(MLIB_PATH_REG_STRU)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of mlib_module.h */