#pragma once

enum STAllocErrorDef
{
	ST_ALLOC_ERROR_SUCCESS = 0, // û���κδ���
	ST_ALLOC_ERROR_ALREADY_INITED = 1,  // �Ѿ���ʼ������
	ST_ALLOC_ERROR_INVALID_PARAMS = 2,  // ��Ч����
	ST_ALLOC_ERROR_UNINITED = 3,  // δ��ʼ��
	ST_ALLOC_SHM_NOT_ALLOCATED = 4, // �����ڴ滹δ�������
	ST_ALLOC_SHM_INFO_NOT_MATCH = 5, // �����ڴ���Ϣ��ƥ��
	ST_ALLOC_SHM_BUDDY_ERROR = 6, // ���ϵͳ������
	ST_ALLOC_SHM_META_ALLOC_FAILED = 7, // Ԫ���ݷ���ʧ��
};