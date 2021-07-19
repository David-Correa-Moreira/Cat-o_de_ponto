{{-- resources/views/control/projects/registers/index.blade.php --}}

@extends('adminlte::page')

@section('title', 'Registers')

@section('content_header')
    <h1>Usuários</h1>
@stop

@section('content')



<div class="card">
@if(Auth::id() == 1)
    <div class="card-header">
        <a href="{{ route('registers.create')  }}" class="btn btn-primary mb-2">Novo Usuário</a>
    </div>
@endif    
    <div class="card-body">
        <div class="table-responsive">
            <table class="table table-hover">
                <thead>
                <tr>

                    <th scope="col">Nome</th>
                    <th scope="col">Situação</th>
                    <th scope="col">Criação</th>
                    <th class="text-center">Ações</th>
                </tr>
                </thead>
                <tbody>
                    @foreach ($users as $user)
                        <tr>
                            <td>{{ $user->u }}</td>
                            <td>{{ $user->s ? 'Ativo': 'Inativo' }}</td>
                            <td>{{ $user->added_at }}</td>
                            <!-- <td><a href="{{ route('registers.show', $user->id) }}" class="btn btn-info">Ver</a></td>
                            <td>
                                <form action="{{ route('registers.delete', $user->id) }}" method="POST">
                                    @csrf
                                    @method('DELETE')
                                    <button type="submit" class="btn btn-danger">Deletar</button>
                                </form>
                            </td>
                            <td><a href="{{ route('registers.edit', $user->id) }}" class="btn btn-info">Editar</a> </td> -->
                            <td class="text-center">
                                <a href="{{ route('registers.show', $user->id) }}" class="btn btn-info">Ver</a>
                                @if(Auth::id() == 1)
                                    <form action="{{ route('registers.delete', $user->id) }}" method="POST" style="display: inline;">
                                        @csrf
                                        @method('DELETE')
                                        <button type="submit" class="btn btn-danger">Deletar</button>
                                    </form>
                                    <a href="{{ route('registers.edit', $user->id) }}" class="btn btn-info">Editar</a>
                                @endif
                            </td>
                        </tr>
                    @endforeach
    
                </tbody>
            </table>
        </div>
    </div>
    <div class="card-footer">
       <small class="text-muted">{!! $users->links() !!}</small> 
    </div>
</div>

@stop

@section('css')
    <link rel="stylesheet" href="/css/admin_custom.css">
@stop

@section('js')
    <script> console.log('Hi!'); </script>
@stop